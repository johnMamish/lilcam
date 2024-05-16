#include "main.h"
#include "usb_device.h"
#include "cmsis_os.h"
#include "usb_task.h"
#include "camera_management_task.h"
#include "camera_read_task.h"

#include "camera_command.pb.h"
#include "pb_decode.h"

extern QueueHandle_t camera_read_task_config_queue;
extern QueueHandle_t camera_management_task_request_queue;

extern QueueHandle_t usb_cdc_rx_queue;
extern QueueHandle_t usb_request_queue;

#define USB_READ_TASK_BUFSZ 512
osThreadId usbReadTaskHandle;
uint32_t usbReadTaskBuffer[ USB_READ_TASK_BUFSZ ];
osStaticThreadDef_t usbReadTaskControlBlock;

static void handle_camera_read_config(const pb_camera_request_t* pb)
{
    const pb_camera_read_request_t* rr = &pb->request.dcmi_config;
    switch(rr->which_request) {
        case PB_CAMERA_READ_REQUEST_CROP_TAG: {
            camera_read_task_set_size(rr->request.crop.len_x, rr->request.crop.len_y);
            camera_read_task_set_crop(rr->request.crop.start_x, rr->request.crop.start_y,
                                      rr->request.crop.len_x, rr->request.crop.len_y);
            break;
        }

        case PB_CAMERA_READ_REQUEST_PACK_TAG: {
            if (rr->request.pack.pack) camera_read_task_enable_packing();
            else camera_read_task_disable_packing();
            break;
        }

        case PB_CAMERA_READ_REQUEST_DCMI_HALT_TAG: {
            // Note that halting DCMI will block this thread until DCMI has actually been halted,
            // which can take up to 1 camera frame.
            if (rr->request.dcmi_halt.halt) camera_read_task_halt_dcmi();
            else camera_read_task_resume_dcmi();
            break;
        }
    }
}

static void handle_camera_management_request(const pb_camera_request_t* pb)
{
    const pb_camera_management_request_t* mr = &pb->request.camera_management;
    switch(mr->which_request) {
        case PB_CAMERA_MANAGEMENT_REQUEST_REG_WRITE_TAG: {
            camera_management_task_reg_write((uint8_t)mr->request.reg_write.i2c_peripheral_address,
                                             (uint16_t)mr->request.reg_write.register_address,
                                             (uint8_t)mr->request.reg_write.value);
            break;
        }

        case PB_CAMERA_MANAGEMENT_REQUEST_SENSOR_SELECT_TAG: {
            // "true" selects hm01b0, "false" selects hm0360.
            if (mr->request.sensor_select.sensor_select ==
                PB_CAMERA_MANAGEMENT_REQUEST_SENSOR_SELECT_SENSOR_SELECT_E_HM01_B0)
                camera_management_task_sensor_select(true);
            else
                camera_management_task_sensor_select(false);
            break;
        }

        case PB_CAMERA_MANAGEMENT_REQUEST_TRIGGER_CONFIG_TAG: {
            // TODO impl
            break;
        }
    }
}

/**
 * The usb_read_task uses the CDC_Receive_HS callback to get data.
 *
 * The CDC_Receive_HS callback function (which is called from an ISR context) places data into a
 * FreeRTOS queue, usb_cdc_rx_queue. This queue is declared in main.c.
 *
 * Individual bytes are enqueued - which is very inefficient - but because the
 * uplink data rate is so low, I feel that this is an acceptable tradeoff for development /
 * debugging time.
 */
void usb_read_task(void const* args)
{
    typedef struct stream_state {
        uint8_t* buf;
        int len;
    } stream_state_t;

    bool pb_callback(pb_istream_t *stream, uint8_t *buf, size_t count) {
        stream_state_t* s = stream->state;
        if (s->len < count)
            return false; // Not enough data in buffer

        // Copy data from buffer to Nanopb internal buffer and adjust buffer
        memcpy(buf, s->buf, count);
        memmove(s->buf, s->buf + count, s->len - count);
        s->len -= count;
        return true;
    }

    static uint8_t buf[256];
    stream_state_t ss = {.buf = buf, .len = 0};
    int current_buffer_len = -1;
    while (1) {
        uint8_t ch;
        xQueueReceive(usb_cdc_rx_queue, &ch, portMAX_DELAY);

        if (current_buffer_len == -1) {
            current_buffer_len = ch;
        } else {
            // append newly read byte to buffer.
            buf[ss.len++] = ch;

            if (ss.len == current_buffer_len) {
                // Try to decode message if buffer has enough data
                pb_istream_t stream = pb_istream_from_buffer(ss.buf, ss.len);
                stream.callback = pb_callback;
                stream.state = &ss;

                pb_camera_request_t request = PB_CAMERA_REQUEST_INIT_ZERO;
                if (pb_decode(&stream, PB_CAMERA_REQUEST_FIELDS, &request)) {
                    current_buffer_len = -1;
                    ss.len = 0;

                    // TODO: handle message.
                    if (request.which_request == PB_CAMERA_REQUEST_CAMERA_MANAGEMENT_TAG) {
                        handle_camera_management_request(&request);
                    } else if (request.which_request == PB_CAMERA_REQUEST_DCMI_CONFIG_TAG) {
                        handle_camera_read_config(&request);
                    }
                } else {
                    while (1);
                }
            }
            taskYIELD();
        }

    }
}

void usb_task(void const* args)
{
    MX_USB_DEVICE_Init();

    osThreadStaticDef(usbReadTask,
                      usb_read_task,
                      osPriorityNormal,
                      0,
                      USB_READ_TASK_BUFSZ,
                      usbReadTaskBuffer,
                      &usbReadTaskControlBlock);
    usbReadTaskHandle = osThreadCreate(osThread(usbReadTask), NULL);

    while (1)
    {
        // Wait until there's a new buffer to transmit
        usb_write_request_t req;
        xQueueReceive(usb_request_queue, &req, portMAX_DELAY);

        // send the request
        uint8_t rs = CDC_Transmit_HS(req.buf, req.len);

        HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_RESET);
        if ((rs == USBD_FAIL) || (rs == USBD_BUSY)) {
            //camera_read_task_halt_dcmi();
            // HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
        }
    }
}

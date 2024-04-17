#include "main.h"
#include "usb_device.h"
#include "cmsis_os.h"
#include "usb_task.h"
#include "camera_management_task.h"

extern QueueHandle_t usb_cdc_rx_queue;
extern QueueHandle_t usb_request_queue;

#define USB_READ_TASK_BUFSZ 512
osThreadId usbReadTaskHandle;
uint32_t usbReadTaskBuffer[ USB_READ_TASK_BUFSZ ];
osStaticThreadDef_t usbReadTaskControlBlock;


/**
 * Really, we should be using protobufs for this; don't have time.
 */
typedef struct camera_management_request_decoder {
    camera_management_request_t req;
    int bytes_decoded;
    int error;
} camera_management_request_decoder_t;

/**
 * Takes in a single byte and updates the state of the request decoder.
 *
 * returns true if decoding the current byte resulted in a completed camera_management_request_t or
 * if it resulted in an error
 */
bool decode_bytes_to_camera_management_request(camera_management_request_decoder_t* dec)
{
    if (dec->bytes_decoded == 0) {

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
    while (1) {
        char ch;
        xQueueReceive(usb_cdc_rx_queue, &ch, portMAX_DELAY);

        taskYIELD();
    }
}

void usb_task(void const* args)
{
    MX_USB_DEVICE_Init();
/*
    osThreadStaticDef(usbReadTask,
                      usb_read_task,
                      osPriorityNormal,
                      0,
                      USB_READ_TASK_BUFSZ,
                      usbReadTaskBuffer,
                      &usbReadTaskControlBlock);
    usbReadTaskHandle = osThreadCreate(osThread(usbReadTask), NULL);
*/
    while (1)
    {
        // Wait until there's a new buffer to transmit
        usb_write_request_t req;
        xQueueReceive(usb_request_queue, &req, portMAX_DELAY);

        // send the request
        uint8_t rs = CDC_Transmit_HS(req.buf, req.len);

        HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_RESET);
        if ((rs == USBD_FAIL) || (rs == USBD_BUSY)) {
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
        }
    }
}

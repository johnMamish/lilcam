#include "camera_read_task.h"
#include "usb_task.h"
#include "hm01b0_init_bytes.h"

#define __unused __attribute__((unused))

#include <string.h>

////////////////////////////////////////////////////////////////
// FreeRTOS includes
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "portmacro.h"

SemaphoreHandle_t camera_frame_ready_semaphore;
StaticSemaphore_t camera_frame_ready_semaphore_buffer;

extern I2C_HandleTypeDef hi2c1;
extern DCMI_HandleTypeDef hdcmi;

static volatile DCMI_TypeDef* dcmi __attribute__((unused)) = DCMI;
static volatile DMA_TypeDef* dma2 __attribute__((unused)) = DMA2;
static volatile DMA_Stream_TypeDef* dma2_stream7 __attribute__((unused)) = DMA2_Stream7;

volatile int framestart_flag = 0;

// queues for IPC are declared and initialized in main
extern QueueHandle_t camera_read_task_config_queue;
extern QueueHandle_t usb_request_queue;

// raw buffer to hold bytes recieved directly from camera
#define RAWBUF_WIDTH  (320 * 2)
#define RAWBUF_HEIGHT (30)
uint8_t camera_rawbuf[2][RAWBUF_WIDTH * RAWBUF_HEIGHT] = { 0 };

// buffer to hold properly packed pixels for usb xfer
#define PACKEDBUF_WIDTH (320)
#define PACKEDBUF_HEIGHT (30)
uint8_t camera_packedbuf[2][PACKEDBUF_WIDTH * (PACKEDBUF_HEIGHT + 1)] = { 0 };

// This C file is directly included because it only includes statically defined stuff for this file.
// Just seperated it out into a different file for readability.
#include "camera_read_task_util.hc"

TaskHandle_t footask_handle;
int framecount = 0;
void DCMI_IRQHandler()
{
    framestart_flag = 1;
    DCMI->ICR = (1 << 3);

    HAL_GPIO_TogglePin(led0_GPIO_Port, led0_Pin);

    //BaseType_t wake_higher_priority_task = pdFALSE;
    //vTaskNotifyGiveFromISR(footask_handle, &wake_higher_priority_task);
    //portYIELD_FROM_ISR(wake_higher_priority_task);
}

void DMA2_Stream7_IRQHandler()
{
    static BaseType_t higher_priority_task_woken;

    // Clear the "transfer complete" interrupt flag.
    DMA2->HIFCR = DMA_HIFCR_CTCIF7;

    // Wake up tasks waiting for an audio buffer to become ready.
    higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(camera_frame_ready_semaphore, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

/**
 * The camera read task has no awareness of which camera is connected to it or how that camera is
 * connected. All it does is read images of the size its asked to and send them as a stream to USB.
 *
 * All camera setup is handled by camera_management_task, which also tells this task what image
 * dimensions to expect.
 */
camera_read_state_t camera_state;
void camera_read_task(void const* args)
{
    camera_frame_ready_semaphore = xSemaphoreCreateBinaryStatic(&camera_frame_ready_semaphore_buffer);

    // camera_read_task doesn't have control over how large incoming frames are, instead it needs
    // to be told by camera_management_task how large it should expect incoming frames to be.
    // This variable keeps track of those active camera settings.
    init_camera_read_state(&camera_state);

    // enable DMA clock
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_DCMI_CLK_ENABLE();

    // Setup DCMI interrupt mask. DCMI interrupt is used to keep track of frame count.
    DCMI->IER = (1 << 3);

    dma_setup_xfer();

    osDelay(1);

    while (1) {
        // This logic assumes that - when the DCMI is being halted - the CAPTURE bit will be
        // cleared before the newly finished DMA xfer is fully processed.
        // This is basically guaranteed, but if it were violated, would result in a race condition.
        if (!camera_state.halted) {
            // wait til DMA is finished
            xSemaphoreTake(camera_frame_ready_semaphore, portMAX_DELAY);

            // we just got some new bytes; they arrive from the hm01b0 in nibbles, so repack them
            int backbuf_idx = (DMA2_Stream7->CR & (1 << 19)) ? 0 : 1;

            // If it's the first line of a frame, then pack in a synchronization line.
            uint8_t* packedbuf = camera_packedbuf[backbuf_idx];
            uint32_t buflen = PACKEDBUF_WIDTH * PACKEDBUF_HEIGHT;
            if (framestart_flag) {
                memcpy(packedbuf, magic, 320);
                packedbuf = packedbuf + 320;
                buflen += 320;
                framestart_flag = 0;
            }

            uint8_t* rawbuf = camera_rawbuf[backbuf_idx];

            // copy bytes from rawbuf to target buffer, packing them from nybbles to bytes if
            // appropriate.
            if (camera_state.pack) {
                for (int i = 0; i < (PACKEDBUF_WIDTH * PACKEDBUF_HEIGHT); i++) {
                    const uint8_t msn = rawbuf[(2 * i) + 1] & 0x0f;
                    const uint8_t lsn = rawbuf[(2 * i) + 0] & 0x0f;
                    packedbuf[i] = ((msn << 4) | (lsn << 0));
                }
            } else {
                // TODO: memcpy()
            }

            // Tell USB that we've got new data for it.
            usb_write_request_t req = {.buf = (void*)camera_packedbuf[backbuf_idx], .len = buflen};
            if (xQueueSendToBack(usb_request_queue, (const void*)&req, 0) == pdTRUE) {
                HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_RESET);
            } else {
                // if we can't push to the queue just drop it and flash an error led
                HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_SET);
            }

            // toggle the pin as a sign of life
            HAL_GPIO_TogglePin(led0_GPIO_Port, led0_Pin);
        }

        // If we have a request to change the camera configuration, process it now.
        // Note that if we're halted, we add in a short delay so as to not spinlock.
        // Also note that if a DCMI halt is pending, do not try to process any requests.
        TickType_t wait_time = camera_state.halted ? 5 : 0;
        camera_read_config_t req;
        if (!camera_state.halt_pending &&
            xQueueReceive(camera_read_task_config_queue, &req, wait_time)) {
            switch (req.config_type) {
                case CAMERA_READ_CONFIG_SETSIZE: {
                    camera_state.width = req.params.image_dims.width;
                    camera_state.height = req.params.image_dims.height;
                    dcmi_crop_set(&camera_state);
                    break;
                }

                case CAMERA_READ_CONFIG_SETCROP: {
                    camera_state.start_x = req.params.crop_dims.start_x;
                    camera_state.start_y = req.params.crop_dims.start_y;
                    camera_state.len_x   = req.params.crop_dims.len_x;
                    camera_state.len_y   = req.params.crop_dims.len_y;
                    dcmi_crop_set(&camera_state);
                    break;
                }

                case CAMERA_READ_CONFIG_SETPACKING: {
                    camera_state.pack = req.params.pack_options.pack;
                    break;
                }

                case CAMERA_READ_CONFIG_HALT: {
                    camera_state.halt_callback = req.params.halt_options.halt_callback;
                    camera_state.halt_callback_user = req.params.halt_options.halt_callback_user;

                    if (req.params.halt_options.halt) {
                        // clear the DCMI's CAPTURE bit. Note that "During normal operation, if the
                        // CAPTURE bit is cleared, the DCMI captures until the end of the frame.
                        DCMI->CR &= ~(1 << 0);
                        camera_state.halt_pending = 1;
                    } else {
                        // start DCMI back up and alert other processes that it's started.
                        dcmi_restart();
                        camera_state.halted = 0;

                        if (camera_state.halt_callback) {
                            camera_state.halt_callback(camera_state.halt_callback_user);
                        }
                    }
                    break;
                }
            }
        }

        // Update halt status according to DCMI status bits.
        if (camera_state.halt_pending && ((DCMI->CR & (1 << 0)) == 0)) {
            // DCMI just halted after a halt was pending.
            camera_state.halt_pending = 0;
            camera_state.halted = 1;

            // Disable DCMI so that DCMI settings can be changed.
            dcmi_disable();

            // Let other processes know that DCMI has been disabled so we can start changing camera
            // settings.
            if (camera_state.halt_callback) {
                camera_state.halt_callback(camera_state.halt_callback_user);
            }
        }
    }
}

/**
 * utility function that notifies a task.
 */
static void notifyme(void** user)
{
    TaskHandle_t t = (TaskHandle_t)(*user);
    xTaskNotify(t, 0, eNoAction);
}

void camera_read_task_enqueue_config(const camera_read_config_t* req)
{
    xQueueSendToBack(camera_read_task_config_queue, (const void*)req, portMAX_DELAY);
}

void camera_read_task_set_size(int width, int height)
{
    camera_read_config_t req = {
        .config_type = CAMERA_READ_CONFIG_SETSIZE,
        .params.image_dims = {width, height}
    };
    xQueueSendToBack(camera_read_task_config_queue, (const void*)&req, portMAX_DELAY);
}

void camera_read_task_set_crop(int start_x, int start_y, int len_x, int len_y)
{
    camera_read_config_t req = {
        .config_type = CAMERA_READ_CONFIG_SETCROP,
        .params.crop_dims = {start_x, start_y, len_x, len_y}
    };
    xQueueSendToBack(camera_read_task_config_queue, (const void*)&req, portMAX_DELAY);
}

void camera_read_task_enable_packing()
{
    camera_read_config_t req = {
        .config_type = CAMERA_READ_CONFIG_SETPACKING,
        .params.pack_options = {true}
    };
    xQueueSendToBack(camera_read_task_config_queue, (const void*)&req, portMAX_DELAY);
}

void camera_read_task_disable_packing()
{
    camera_read_config_t req = {
        .config_type = CAMERA_READ_CONFIG_SETPACKING,
        .params.pack_options = {false}
    };
    xQueueSendToBack(camera_read_task_config_queue, (const void*)&req, portMAX_DELAY);
}

void camera_read_task_halt_dcmi()
{
    void* thistask = xTaskGetCurrentTaskHandle();
    camera_read_config_t req = {
        .config_type = CAMERA_READ_CONFIG_HALT,
        .params.halt_options = {notifyme, &thistask, true}
    };
    xQueueSendToBack(camera_read_task_config_queue, (const void*)&req, portMAX_DELAY);
}

/**
 * Asks the camera read task to resume the DCMI interface. Blocks until this is done.
 */
void camera_read_task_resume_dcmi()
{
    void* thistask = xTaskGetCurrentTaskHandle();
    camera_read_config_t req = {
        .config_type = CAMERA_READ_CONFIG_HALT,
        .params.halt_options = {NULL, NULL, false}
    };
    xQueueSendToBack(camera_read_task_config_queue, (const void*)&req, portMAX_DELAY);
}

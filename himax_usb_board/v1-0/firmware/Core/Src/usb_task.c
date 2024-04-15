#include "main.h"
#include "usb_device.h"
#include "cmsis_os.h"
#include "usb_task.h"

extern QueueHandle_t usb_request_queue;

#define USB_READ_TASK_BUFSZ 512
osThreadId usbReadTaskHandle;
uint32_t usbReadTaskBuffer[ USB_READ_TASK_BUFSZ ];
osStaticThreadDef_t usbReadTaskControlBlock;

/*
void usb_read_task(void const* args)
{
    while (1) {
        static uint8_t buf[1024];
        int len = 1024;
        uint8_t rs = CDC_Receive_HS(buf, &len);

        if (len > 0) {
            if (buf[0] == 'a') {
                HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_SET);
            } else if (buf[0] == 'b') {
                HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_RESET);
            }
        }
        taskYIELD();
    }
}
*/

/**
 *
 */
void usb_task(void const* args)
{
    tusb_init();

    while (1)
    {
        // Handle non-ISR tinyusb tasks.
        // Note that transmission of data is handled by code 'inside' of the tusb stack.
        tud_task();

        // TODO: Handle recieved camera config commands.
    }
}

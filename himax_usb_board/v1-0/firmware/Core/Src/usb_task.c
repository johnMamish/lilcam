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

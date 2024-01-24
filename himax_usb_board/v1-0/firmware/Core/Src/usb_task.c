#include "main.h"
#include "usb_device.h"
#include "cmsis_os.h"
#include "usb_task.h"

extern QueueHandle_t usb_request_queue;

void usb_task(void const* args)
{
    MX_USB_DEVICE_Init();

    for(;;)
    {
        // Wait until there's a new buffer
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

#include "system_task.h"

void system_task(void const* args)
{
    int resetcount = 0;
    while (1) {
        osDelay(100);

        // if the button is held for a long time, disable interrupts, disable USB, and jump to
        // bootloader
        if (!HAL_GPIO_ReadPin(user_button_GPIO_Port, user_button_Pin)) {
            resetcount++;
        } else {
            resetcount = 0;
        }

        if (resetcount > 50) {
            // if we held down the button for 5 seconds, enter the bootloader.
            // TODO: figure out how to properly deinitialize usb HS.
            uint32_t USBx_BASE = (uint32_t)USB_OTG_HS;
            USBx_DEVICE->DCTL |= USB_OTG_DCTL_SDIS;
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
            osDelay(100);

            // disable interrupts
            __disable_irq();

            // Individually disable each interrupt. Can't find "num vectors" in driver files so
            // I'm manually hardcoding the value found in the NVIC section of the datasheet.
            const int NVECTORS = 98;
            for (uint32_t i = 0; i < NVECTORS; i++) {
                NVIC_DisableIRQ(i);
            }

            /**
             * Step: Disable RCC, set it to default (after reset) settings
             *       Internal clock, no PLL, etc.
             */
            //HAL_RCC_DeInit();
            //HAL_DeInit();  // add by ctien

            // Step: Disable systick timer and reset it to default values
            SysTick->CTRL = 0;
            SysTick->LOAD = 0;
            SysTick->VAL = 0;

            // jump to bootloader

            void (*SysMemBootJump)(void);
            SysMemBootJump = (void (*)(void))(0x0800ffff);
            SysMemBootJump();
        } else {
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_RESET);
        }
    }
}

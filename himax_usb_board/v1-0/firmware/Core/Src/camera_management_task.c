#include "camera_management_task.h"
#include "usb_task.h"
#include "hm01b0_init_bytes.h"
#include "hm0360_init_bytes.h"

#define __unused __attribute__((unused))

////////////////////////////////////////////////////////////////
// FreeRTOS includes
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "queue.h"
#include "list.h"
#include "portmacro.h"

/**
 * Triggers
 *
 * The easiest way to do this is to:
 *   - by default, the camera does not record frames and instead waits for a trigger.
 *   - if the user button is short-pressed, the camera starts to generate triggers and also record
 *     frames.
 *   - Not ideal, but inter-camera triggers and mcu -> sensor triggers will all be processed by
 *     software interrupts.
 *   - Inter-camera triggers will be generated by software as well.
 */

// PF9 is the external trigger signal.
void EXTI9_5_IRQHandler() {
    if (EXTI->PR & (1 << 9)) {
        // send trigger pulse to hm01b0 by enabling tim5 for a one-shot
        TIM5->CR1 |= (1 << 0);

        // Clear exti 9
        EXTI->PR |= (1 << 9);

        // toggle LED1
        // HAL_GPIO_TogglePin(led1_GPIO_Port, led1_Pin);
    }
}

// TODO: if we are a trigger generator, then have a timer interrupt that triggers the other lilcam and
// also our image sensor

extern QueueHandle_t camera_management_task_request_queue;

#define TG_TASK_BUFSZ 512
osThreadId TGTaskHandle;
uint32_t TGTaskBuffer[ TG_TASK_BUFSZ ];
osStaticThreadDef_t TGTaskControlBlock;

static volatile TIM_TypeDef* tim5 = TIM5;

/**
 * This task waits for a user button press. If the user quick-presses button 'A', then the camera
 * switches between triggered and trigger-gen mode.
 *
 * Led 1 mirrors trigger status.
 */
void trigger_generation_task(void* args)
{
    const int MODE_TRIGGERED = 0;
    const int MODE_TRIGGER_GEN = 1;

    int button_prev = !HAL_GPIO_ReadPin(user_button_GPIO_Port, user_button_Pin);
    int mode = MODE_TRIGGER_GEN;
    while (1) {
        // todo: make rate programmable.
        // todo: don't launch trigger unless hm01b0 is ready to read another frame
        osDelay(100);
        if (mode == MODE_TRIGGER_GEN) {
            // send trigger to other microcontroller
            HAL_GPIO_WritePin(gpio4_GPIO_Port, gpio4_Pin, GPIO_PIN_SET);

            HAL_GPIO_WritePin(hm01b0_trig_GPIO_Port, hm01b0_trig_Pin, GPIO_PIN_SET);
            //TIM5->CR1 |= (1 << 0);

            // toggle LED1
            //HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_SET);
        }

        osDelay(1);

        if (mode == MODE_TRIGGER_GEN) {
            // send trigger to other microcontroller
            HAL_GPIO_WritePin(gpio4_GPIO_Port, gpio4_Pin, GPIO_PIN_RESET);
            //HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_RESET);
        }

        // check button
        int button_now = !HAL_GPIO_ReadPin(user_button_GPIO_Port, user_button_Pin);
        if (button_prev && !button_now) {
            mode = (mode == MODE_TRIGGERED) ? MODE_TRIGGER_GEN : MODE_TRIGGERED;
        }
    }
}

extern I2C_HandleTypeDef hi2c1;

static void init_internal_hw_trig()
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // setup TIM5 channel 2 on PA1 to trigger hm01b0
#if 0
    __HAL_RCC_TIM5_CLK_ENABLE();

    // setup hm01b0 trigger pin as TIM5 CH2
    GPIO_InitStruct.Pin = hm01b0_trig_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM5;
    HAL_GPIO_Init(hm01b0_trig_GPIO_Port, &GPIO_InitStruct);

    // timer in one-pulse mode
    TIM5->CR1 |= (1 << 3);

    // put channel 2 in PWM1 Mode
    TIM5->CCMR1 &= ~((0xfful << 8) | (1ul << 24));
    TIM5->CCMR1 |=  (6 << 12);
    TIM5->CCER  &= ~(0xful << 4);
    TIM5->CCER  |=  (1 << 5) | (1 << 4);

    // pulse for a bit.
    // Note that channel is inverted because "idle state" for the timer is '0' and the one-shot
    // doesn't support
    // the CCR2 = 1 causes a very short (~10s of ns) delay
    TIM5->CCR2 = 1;
    TIM5->ARR = 5000;
#endif
    // setup gpio3 (trigger in) as input
    GPIO_InitStruct.Pin = gpio3_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    // setup exti 9 to be triggered by PF9.
    // P'F' requires '5'. (A = '0', B = '1', etc)
    SYSCFG->EXTICR[9 >> 2] &= ~(0b1111 << ((9 & 0x03) * 4));
    SYSCFG->EXTICR[9 >> 2] |=  (0b0101 << ((9 & 0x03) * 4));

    EXTI->RTSR |= (1 << 9);           // Enable rising edge on exti trigger 9
    EXTI->PR   |= (1 << 9);           // Clear exti channel 9
    EXTI->IMR  |= (1 << 9);           // Unmask exti 9

    // Enable exti[9:5] irq with a high priority (1)
    HAL_NVIC_SetPriority(EXTI9_5_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void hm01b0_i2c_reset();
void hm01b0_i2c_init();

void hm0360_i2c_init();


void camera_management_task(void const* args)
{
    //init_internal_hw_trig();

    // tim2 channel 3 is hm01b0's mclk. Drive it at 12MHz.
    // tim2 runs at 96MHz. we need to divide it by 8
    TIM2->CCER = (1 << 8);
    TIM2->CR1 |= (1 << 0);

    // set camera select to choose hm01b0
    HAL_GPIO_WritePin(camera_select_GPIO_Port, camera_select_Pin, GPIO_PIN_SET);

    // camera read task starts halted.
    // set up camera read task to expect 320x240 image with packing (640x240).
    //camera_read_task_set_size(320*2, 240);
    //camera_read_task_set_crop(2*2, 2*2, 320 * 2, 240);
    //camera_read_task_set_crop(2, 2, 320, 240);
    //camera_read_task_set_size(320, 240);
    //camera_read_task_enable_packing();

    // let the MCLK run for a little bit and then do an i2c reset
    osDelay(1);
    hm01b0_i2c_reset();
    hm01b0_i2c_init();

    // select and enable hm0360
    HAL_GPIO_WritePin(camera_select_GPIO_Port, camera_select_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(hm0360_xshut_GPIO_Port, hm0360_xshut_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(hm0360_xsleep_GPIO_Port, hm0360_xsleep_Pin, GPIO_PIN_SET);

    // TODO: tim xx channel xx is hm0360's mclk. Drive it at xxMHz (?)

    // let the MCLK run for a little bit and then do an i2c reset
    osDelay(1);
    hm0360_i2c_init();

    // camera read task must be enabled by sending a command over serial.
    // DCMI is not enabled by default.

    while (1) {
        // wait for a new request
        camera_management_request_t req;
        xQueueReceive(camera_management_task_request_queue, &req, portMAX_DELAY);

        switch(req.request_type) {
            case CAMERA_MANAGEMENT_TYPE_REG_WRITE: {
                // don't disable dcmi; if a task needs to disable dcmi, it can do it itself by
                // talking directly to the camera read task
                uint8_t buf[3] = {
                    (req.params.reg_write.addr & 0xff00) >> 8,
                    (req.params.reg_write.addr & 0x00ff) >> 0,
                    req.params.reg_write.data
                };
                const uint8_t i2c_addr = req.params.reg_write.peripheral_address << 1;
                __unused HAL_StatusTypeDef err;
                err = HAL_I2C_Master_Transmit(&hi2c1, i2c_addr, buf, 3, 100);

                break;
            }

            case CAMERA_MANAGEMENT_TYPE_SENSOR_SEL: {
                // disable dcmi

                // disable active sensor (necessary? maybe just leave enabled.)

                // switch sensor bus
                if (req.params.sensor_select == CAMERA_MANAGEMENT_SENSOR_SELECT_HM01B0)
                    HAL_GPIO_WritePin(camera_select_GPIO_Port, camera_select_Pin, GPIO_PIN_SET);
                else if (req.params.sensor_select == CAMERA_MANAGEMENT_SENSOR_SELECT_HM0360)
                    HAL_GPIO_WritePin(camera_select_GPIO_Port, camera_select_Pin, GPIO_PIN_RESET);

                // enable inactive sensor (necessary? maybe just leave enabled.)

                // adjust dcmi settings

                // ONLY re-enable dcmi if it was enabled before.

                break;
            }

            case CAMERA_MANAGEMENT_TYPE_TRIGGER_CONFIG: {
                // UNIMPLEMENTED
                break;
            }
        }
    }
}


void hm01b0_i2c_reset()
{
    hm01b0_reg_write_t v = {0x0103, 0xff};
    uint8_t buf[3] = {(v.ui16Reg & 0xff00) >> 8, (v.ui16Reg & 0x00ff) >> 0, v.ui8Val};
    __unused HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c1, 0x24 << 1, buf, 3, 100);

    v.ui16Reg = 0x0103; v.ui8Val = 0x00;
    buf[0] = (v.ui16Reg & 0xff00) >> 8; buf[1] = (v.ui16Reg & 0x00ff) >> 0; buf[2] = v.ui8Val;
    err = HAL_I2C_Master_Transmit(&hi2c1, 0x24 << 1, buf, 3, 100);
}


void hm01b0_i2c_init()
{
    // initialize hm01b0 over i2c
    for (int i = 0; i < sizeof_hm01b0_init_values / sizeof(hm01b0_init_values[0]);) {
        hm01b0_reg_write_t v = hm01b0_init_values[i++];
        uint8_t buf[3] = {(v.ui16Reg & 0xff00) >> 8, (v.ui16Reg & 0x00ff) >> 0, v.ui8Val};

        HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c1, 0x24 << 1, buf, 3, 100);

        if (err) {
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
        }

        taskYIELD();
    }
}


void hm0360_i2c_init()
{
    // initialize hm0360 over i2c
    for (int i = 0; i < sizeof_hm0360_init_values / sizeof(hm0360_init_values[0]);) {
        hm0360_reg_write_t v = hm0360_init_values[i++];
        uint8_t buf[3] = {(v.ui16Reg & 0xff00) >> 8, (v.ui16Reg & 0x00ff) >> 0, v.ui8Val};

        HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c1, 0x35 << 1, buf, 3, 100);

        if (err) {
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
        }

        taskYIELD();
    }
}


void camera_management_task_enqueue_request(const camera_management_request_t* req)
{
    xQueueSendToBack(camera_management_task_request_queue, (const void*)req, portMAX_DELAY);
}

void camera_management_task_reg_write(uint8_t i2c_addr, uint16_t reg_addr, uint8_t data)
{
    camera_management_request_t req;
    req.request_type = CAMERA_MANAGEMENT_TYPE_REG_WRITE;
    req.params.reg_write = (i2c_reg_write_t){i2c_addr, reg_addr, data};
    camera_management_task_enqueue_request(&req);
}

void camera_management_task_sensor_select(bool which)
{
    camera_management_request_t req;
    req.request_type = CAMERA_MANAGEMENT_TYPE_SENSOR_SEL;
    req.params.sensor_select = which ?
        CAMERA_MANAGEMENT_SENSOR_SELECT_HM01B0:
        CAMERA_MANAGEMENT_SENSOR_SELECT_HM0360;
    camera_management_task_enqueue_request(&req);
}

// start trigger generation task
#if 0
    TaskHandle_t trigger_generation_task_handle = xTaskCreateStatic(trigger_generation_task,
                                                                    "trig gen task",
                                                                    TG_TASK_BUFSZ,
                                                                    0,
                                                                    osPriorityNormal,
                                                                    TGTaskBuffer,
                                                                    &TGTaskControlBlock);
#endif

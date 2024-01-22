#include "camera_task.h"
#include "hm01b0_init_bytes.h"

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

#define WIDTH 320
#define HEIGHT 240
uint8_t camerabuf[2][2 * WIDTH * HEIGHT] = { 0 };

/**
 * Some assorted notes:
 *   - for the 640x480 sensor, DMA will need to be done in 2 xfers. max DMA xfer size is 256kB
 *     Well, anyways, we only have space on this mcu to buffer one frame.
 */

static void dma_setup_xfer()
{
    // 1. make sure that the stream is disabled
    if (0) { while (1); }

    // 2. Set the peripheral port register address in the DMA_SxPAR register.
    DMA2_Stream7->PAR = (uint32_t)(&(DCMI->DR));

    // 3. set the target memory addres in the M0AR / M1AR (if used) address.
    DMA2_Stream7->M0AR = (uint32_t)(camerabuf[0]);
    DMA2_Stream7->M1AR = (uint32_t)(camerabuf[1]);

    // 4. Configure the total number of data items to be transferred in the NDTR register.
    //DMA2_Stream7->NDTR = sizeof(camerabuf[0]);
    DMA2_Stream7->NDTR = ((uint16_t)(320 * 240 * 2 / 4));

    // DMA2, stream 7, channel 1 is DCMI.
    // 5. Select the DMA channel (request) using CHSEL[2:0] in DMA_SxCR
    // 6. If the peripheral is the flow-controller, set the PFCTRL bit in the DMA_SxCR reg.
    // 7. Configure the stream priority (nah.)
    // 9. Configure other fields of SxCR reg.
    DMA2_Stream7->CR = ((0b001 << 25) |       // select channel 1
                        ( 0b00 << 23) |       // memory burst is a single beat
                        ( 0b00 << 21) |       // peripheral burst is a single beat
                        (  0b0 << 19) |       // current target is memory 0
                        (  0b1 << 18) |       // double buffer mode
                        ( 0b10 << 16) |       // priority level
                        (  0b0 << 15) |       // peripheral incr offset (d.c. because no periph incr)
                        ( 0b10 << 13) |       // memory data size: 32b
                        ( 0b10 << 11) |       // peripheral data size: 32b
                        (  0b1 << 10) |       // memory increment mode: memory is incremented
                        (  0b0 <<  9) |       // peripheral increment mode: peripheral ptr is fixed
                        (  0b0 <<  8) |       // circular mode disabled
                        ( 0b00 <<  6) |       // peripheral-to-memory direction
                        (  0b0 <<  5) |       // DMA does 'flow control'; it decides xfer length.
                        (    1 <<  4));       // Transfer complete interrupt enabled

    // 8. configure the FIFO usage (n/a ?)
    DMA2_Stream7->FCR = 0;

    // 10. activate the stream by setting the enable bit
    DMA2_Stream7->CR |= (1 << 0);

    // clear transfer complete interrupt status
    DMA2->HIFCR = DMA_HIFCR_CTCIF7;

    HAL_NVIC_SetPriority(DMA2_Stream7_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(DMA2_Stream7_IRQn);
}

void DCMI_IRQHandler()
{
    DCMI->ICR = 1 | (1 << 3) | (1 << 4);
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

void camera_task(void const* args)
{
    camera_frame_ready_semaphore = xSemaphoreCreateBinaryStatic(&camera_frame_ready_semaphore_buffer);

    // enable DMA clock
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_DCMI_CLK_ENABLE();

    // make sure that DCMI and DMA are enabled before the hm01b0 starts sending images.
    // NOTE: the 'active' state of hsync and vsync is 0. hsync and vsync are considered active when
    // data is invalid, not when data is valid.
    DCMI->IER = (1 << 0) | (1 << 3);
    DCMI->ICR = (1 << 0) | (1 << 3);
    DCMI->CR |= (1 << 14);
    dma_setup_xfer();

    DCMI->CR |= (1 << 0);

    // for starters, set camera select to choose hm01b0
    HAL_GPIO_WritePin(camera_select_GPIO_Port, camera_select_Pin, GPIO_PIN_SET);

    // tim2 channel 3 is hm01b0's mclk. Drive it at 12MHz.
    // tim2 runs at 96MHz. we need to divide it by 8
    TIM2->CCER = (1 << 8);
    TIM2->CR1 |= (1 << 0);

    // let the MCLK run for a little bit before trying to do anything
    osDelay(200);

    // reset hm01b0
    {
        hm01b0_reg_write_t v = {0x0103, 0xff};
        uint8_t buf[3] = {(v.ui16Reg & 0xff00) >> 8, (v.ui16Reg & 0x00ff) >> 0, v.ui8Val};
        HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c1, 0x24 << 1, buf, 3, 100);

        v.ui16Reg = 0x0103; v.ui8Val = 0x00;
        buf[0] = (v.ui16Reg & 0xff00) >> 8; buf[1] = (v.ui16Reg & 0x00ff) >> 0; buf[2] = v.ui8Val;
        err = HAL_I2C_Master_Transmit(&hi2c1, 0x24 << 1, buf, 3, 100);
    }

    osDelay(5);

    // initialize hm01b0 over i2c
    for (int i = 0; i < sizeof_hm01b0_init_values / sizeof(hm01b0_init_values[0]);) {
        hm01b0_reg_write_t v = hm01b0_init_values[i++];
        uint8_t buf[3] = {(v.ui16Reg & 0xff00) >> 8, (v.ui16Reg & 0x00ff) >> 0, v.ui8Val};

        HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c1, 0x24 << 1, buf, 3, 100);

        HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_RESET);
        if (err) {
            HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_SET);
        }

        taskYIELD();
    }

    while (1) {
        // DMA xfer should already be setup; doing ping-pong operation
        // signal to usb that the frontbuffer is ready for transmission
        volatile uint32_t* backptr = (DMA1_Stream0->CR & (1 << 19)) ? &DMA1_Stream0->M0AR : &DMA1_Stream0->M1AR;

        // at this point, some time should pass before the next frame starts.
        // wait til DMA is finished
        xSemaphoreTake(camera_frame_ready_semaphore, portMAX_DELAY);

        // toggle LED0 as sign of life.
        HAL_GPIO_TogglePin(led0_GPIO_Port, led0_Pin);
    }
}

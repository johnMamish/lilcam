#include "camera_task.h"
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

// raw buffer to hold
#define RAWBUF_WIDTH  (320 * 2)
#define RAWBUF_HEIGHT (60)
uint8_t camera_rawbuf[2][RAWBUF_WIDTH * RAWBUF_HEIGHT] = { 0 };

// buffer to hold properly packed pixels for usb xfer
#define PACKEDBUF_WIDTH (320)
#define PACKEDBUF_HEIGHT (60)
uint8_t camera_packedbuf[2][PACKEDBUF_WIDTH * (PACKEDBUF_HEIGHT + 1)] = { 0 };

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
    DMA2_Stream7->M0AR = (uint32_t)(camera_rawbuf[0]);
    DMA2_Stream7->M1AR = (uint32_t)(camera_rawbuf[1]);

    // 4. Configure the total number of data items to be transferred in the NDTR register.
    //DMA2_Stream7->NDTR = sizeof(camerabuf[0]);
    //DMA2_Stream7->NDTR = ((uint16_t)(324 * 244 * 2 / 4));
    DMA2_Stream7->NDTR = ((uint16_t)(RAWBUF_WIDTH * RAWBUF_HEIGHT / 4));

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
    framestart_flag = 1;
    DCMI->ICR = (1 << 4) | (1 << 3);

    HAL_GPIO_TogglePin(led0_GPIO_Port, led0_Pin);
}

volatile int xfer_count = 0;
void DMA2_Stream7_IRQHandler()
{
    static BaseType_t higher_priority_task_woken;

    xfer_count = (xfer_count + 1) % 4;

    // Clear the "transfer complete" interrupt flag.
    DMA2->HIFCR = DMA_HIFCR_CTCIF7;

    // Wake up tasks waiting for an audio buffer to become ready.
    higher_priority_task_woken = pdFALSE;
    xSemaphoreGiveFromISR(camera_frame_ready_semaphore, &higher_priority_task_woken);
    portYIELD_FROM_ISR(higher_priority_task_woken);
}

// TODO by using LFSR or similar we could save 320B in flash if we had to
const uint8_t magic[320] = {
    0x48, 0xc1, 0x20, 0xa3, 0xb3, 0x94, 0x74, 0xc2, 0xa6, 0xb7,
    0xc7, 0x76, 0x1e, 0x07, 0x4c, 0x07, 0xde, 0x00, 0x46, 0x44,
    0x05, 0xc3, 0x96, 0xe6, 0xfe, 0x38, 0xc3, 0x2f, 0x1e, 0x5a,
    0x96, 0xb8, 0x8c, 0x4e, 0x1a, 0x2a, 0x9c, 0xc9, 0xf4, 0x67,
    0x29, 0xbf, 0x5c, 0x55, 0xe6, 0x25, 0xfa, 0x4a, 0xaa, 0x17,
    0xd0, 0x54, 0xaf, 0x0b, 0xd6, 0x4c, 0xdc, 0x8d, 0x63, 0x19,
    0x2f, 0x20, 0xcb, 0x5d, 0xd5, 0x7b, 0x45, 0x5a, 0xb7, 0x36,
    0x1e, 0xdb, 0x7f, 0xde, 0x6e, 0x5e, 0xc1, 0x2c, 0x42, 0x43,
    0x71, 0x97, 0x69, 0xb1, 0x6a, 0x82, 0x78, 0x2d, 0xab, 0xab,
    0xf6, 0x33, 0xef, 0x9f, 0x7f, 0x59, 0xa1, 0xbd, 0x65, 0x9e,
    0x91, 0x29, 0xdf, 0x20, 0x91, 0x07, 0x23, 0x32, 0x1d, 0x2a,
    0xf8, 0xeb, 0x95, 0xc0, 0xe9, 0x6a, 0x90, 0x53, 0x70, 0x89,
    0x11, 0xb3, 0x7b, 0x21, 0xb6, 0x7c, 0xb6, 0xc4, 0x80, 0x47,
    0xba, 0x52, 0x58, 0xcb, 0x35, 0x0a, 0x1b, 0x0a, 0x89, 0xe1,
    0x8c, 0xf7, 0xef, 0xc9, 0x13, 0x4a, 0x05, 0x48, 0x97, 0x0e,
    0x29, 0xf0, 0xc0, 0xcd, 0x15, 0xd7, 0x90, 0x08, 0x36, 0x86,
    0xee, 0x8e, 0x23, 0x37, 0xde, 0x0e, 0xe6, 0xee, 0x78, 0x61,
    0xfe, 0x42, 0xc8, 0xc4, 0x1d, 0x9f, 0xf3, 0xd2, 0x13, 0x98,
    0xc9, 0x34, 0x27, 0xc4, 0xd2, 0x12, 0x8c, 0x1a, 0xdd, 0xbf,
    0x28, 0xf0, 0xd3, 0xdd, 0xdf, 0x49, 0x90, 0x3e, 0xc6, 0xb0,
    0x5a, 0xd1, 0x5b, 0xbd, 0x92, 0xe1, 0xd4, 0xa1, 0x35, 0xe9,
    0x60, 0xbe, 0x79, 0x89, 0x28, 0x4e, 0xa0, 0xdd, 0xdc, 0xcb,
    0xc2, 0xb3, 0x7c, 0x6a, 0x48, 0xe2, 0x84, 0x57, 0x95, 0x56,
    0xbf, 0x34, 0xfb, 0x9f, 0xc3, 0x2b, 0x38, 0x57, 0x64, 0x73,
    0xd8, 0xf2, 0xc1, 0xee, 0x97, 0x61, 0x20, 0xf7, 0x26, 0xc1,
    0x4c, 0x69, 0xfb, 0xe2, 0xd2, 0xb3, 0x27, 0x0e, 0xa7, 0xa7,
    0xaf, 0xf9, 0x36, 0xec, 0xa7, 0x64, 0x2a, 0x69, 0xf7, 0xec,
    0x13, 0xdc, 0x53, 0xa3, 0x39, 0x64, 0x85, 0x0e, 0x1d, 0x10,
    0x49, 0x21, 0xe0, 0xc4, 0x9d, 0xdc, 0xaa, 0x11, 0xe6, 0xfc,
    0xca, 0xfe, 0x37, 0x92, 0x3f, 0xed, 0x4e, 0x0c, 0x69, 0x86,
    0x5f, 0x66, 0x59, 0xfc, 0x7a, 0x95, 0x5b, 0xcd, 0x18, 0xca,
    0x86, 0xac, 0xd4, 0xc8, 0xef, 0x8f, 0x89, 0x2b, 0xe6, 0x2d
};

extern QueueHandle_t usb_request_queue;

void camera_task(void const* args)
{
    camera_frame_ready_semaphore = xSemaphoreCreateBinaryStatic(&camera_frame_ready_semaphore_buffer);

    // for starters, set camera select to choose hm01b0
    HAL_GPIO_WritePin(camera_select_GPIO_Port, camera_select_Pin, GPIO_PIN_SET);

    // tim2 channel 3 is hm01b0's mclk. Drive it at 12MHz.
    // tim2 runs at 96MHz. we need to divide it by 8
    TIM2->CCER = (1 << 8);
    TIM2->CR1 |= (1 << 0);

    // let the MCLK run for a little bit before trying to do anything
    osDelay(10);

    // reset hm01b0
    {
        hm01b0_reg_write_t v = {0x0103, 0xff};
        uint8_t buf[3] = {(v.ui16Reg & 0xff00) >> 8, (v.ui16Reg & 0x00ff) >> 0, v.ui8Val};
        __unused HAL_StatusTypeDef err = HAL_I2C_Master_Transmit(&hi2c1, 0x24 << 1, buf, 3, 100);

        v.ui16Reg = 0x0103; v.ui8Val = 0x00;
        buf[0] = (v.ui16Reg & 0xff00) >> 8; buf[1] = (v.ui16Reg & 0x00ff) >> 0; buf[2] = v.ui8Val;
        err = HAL_I2C_Master_Transmit(&hi2c1, 0x24 << 1, buf, 3, 100);
    }

    // enable DMA clock
    __HAL_RCC_DMA2_CLK_ENABLE();
    __HAL_RCC_DCMI_CLK_ENABLE();

    osDelay(1);

    // output of HM01B0 is 324 x 244. We want to crop the border of 4 dummy pixels.
    DCMI->CWSIZER = (239 << 16) | (((320 * 2) - 1) << 0);    // image is 320 x 240
    DCMI->CWSTRTR = (  2 << 16) | (  4 << 0);    // 2 pixel border around edge.
    DCMI->CR |= (1 << 2);                   // enable crop feature

    // make sure that DCMI and DMA are enabled before the hm01b0 starts sending images.
    DCMI->CR &= 0xffe0bfff;
    DCMI->IER = (1 << 3);
    DCMI->CR |= (1 << 14);

    dma_setup_xfer();

    // enable DCMI IRQ
    HAL_NVIC_SetPriority(DCMI_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(DCMI_IRQn);

    DCMI->CR |= (1 << 0);

    osDelay(1);

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
    HAL_GPIO_WritePin(led2_GPIO_Port, led2_Pin, GPIO_PIN_RESET);

    while (1) {
        // wait til DMA is finished
        xSemaphoreTake(camera_frame_ready_semaphore, portMAX_DELAY);

        // we just got some new bytes; they arrive from the hm01b0 in nibbles, so repack them
        int backbuf_idx = (DMA2_Stream7->CR & (1 << 19)) ? 0 : 1;

        // TODO: if it's the first line of a frame, then pack in a synchronization line.
        uint8_t* packedbuf = camera_packedbuf[backbuf_idx];
        uint32_t buflen = PACKEDBUF_WIDTH * PACKEDBUF_HEIGHT;
        if (xfer_count == 1) {
            memcpy(packedbuf, magic, 320);
            packedbuf = packedbuf + 320;
            buflen += 320;
        }

        //if (framestart_flag) {
        if (xfer_count == 0) {
            HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_SET);
        } else {
            HAL_GPIO_WritePin(led1_GPIO_Port, led1_Pin, GPIO_PIN_RESET);
        }


        uint8_t* rawbuf = camera_rawbuf[backbuf_idx];

        for (int i = 0; i < (PACKEDBUF_WIDTH * PACKEDBUF_HEIGHT); i++) {
            const uint8_t msn = rawbuf[(2 * i) + 1] & 0x0f;
            const uint8_t lsn = rawbuf[(2 * i) + 0] & 0x0f;
            packedbuf[i] = ((msn << 4) | (lsn << 0));
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
}

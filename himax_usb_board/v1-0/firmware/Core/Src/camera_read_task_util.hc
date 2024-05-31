/**
 * Internal-use struct that keeps track of camera state
 */
typedef struct camera_read_state {
    // width and height of full image
    uint16_t width, height;

    // crop settings for DCMI
    uint16_t start_x, start_y, len_x, len_y;

    // Is packing into nybbles enabled?
    // Note that this doesn't affect width and height calculations: if a 320x240 image
    // is being sent nybble-wise, then the controlling process needs to specify that
    // width = 320 * 2 = 640.
    uint8_t pack;

    // If this is true, then DCMI is currently halted.
    // DCMI can be halted in order to update some camera settings that shouldn't be updated while
    // the camera is operating. This ensures that DCMI doesn't get desynced and that camera settings
    // are only changed on a frame boundary.
    bool halt_pending, halted;

    // Counter of how many bytes have been transmitted this frame and also how many bytes are
    // in each packed buffer. This lets us make sure that new frame markers are inserted at the
    // right place
    uint32_t byte_count;
    uint32_t packed_buffer_size;         // TODO: currently unused

    // this user-defined callback function is called when the DCMI is brought into or out of halt
    void (*halt_callback)(void**);
    void** halt_callback_user;
} camera_read_state_t;

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

/**
 * utility function that initializes camera_read_state_t to default values
 */
static void init_camera_read_state(camera_read_state_t* crs)
{
    crs->width = -1;
    crs->height = -1;

    crs->start_x = 0;
    crs->start_y = 0;
    crs->len_x = 0;
    crs->len_y = 0;

    crs->pack = 1;
    crs->halt_pending = 0;
    crs->halted = 1;

    crs->byte_count = 0;
    // TODO: update this value to reflect
    crs->packed_buffer_size = CAMERA_BUF_WIDTH * CAMERA_BUF_HEIGHT;

    crs->halt_callback = NULL;
    crs->halt_callback_user = NULL;
}

/**
 * Updates the DCMI peripheral's size registers according to the given camera_read_state struct.
 *
 * The DCMI peripheral should be halted when this function is called.
 */
static void dcmi_crop_set(const camera_read_state_t* crs)
{
    // handle cropping
    bool crop_enabled = !((crs->width == crs->len_x) &&
                          (crs->height == crs->len_y) &&
                          (crs->start_x == 0) &&
                          (crs->start_y == 0));
    if (crop_enabled) {
        // setup crop values
        DCMI->CWSIZER = (((crs->len_y - 1) << 16) | ((crs->len_x - 1) << 0));
        DCMI->CWSTRTR = (crs->start_y << 16) | (crs->start_x << 0);    // 2 pixel border around edge.

        // enable crop feature
        DCMI->CR |= (1 << 2);
    } else {
        // disable crop feature
        DCMI->CR &= ~(1 << 2);
    }
}


/**
 * Utility function that disables the DCMI module so that it's safe to mess with camera settings.
 *
 * Before this function is called, the CAPTURE bit in the DCMI should be cleared.
 */
static void dcmi_disable()
{
    // precondition: DCMI->CR.CAPTURE == 0
    HAL_NVIC_DisableIRQ(DCMI_IRQn);
    DCMI->CR &= ~(1 << 14);
}


/**
 * Re-enables and re-starts DCMI after it's been halted and disabled.
 */
static void dcmi_restart()
{
    DCMI->CR |= (1 << 14);
    HAL_NVIC_SetPriority(DCMI_IRQn, 10, 0);
    HAL_NVIC_EnableIRQ(DCMI_IRQn);
    DCMI->CR |= (1 << 0);
}


static void dma_setup_xfer()
{
    // TODO?: 1. make sure that the stream is disabled
    DMA2_Stream7->CR &= ~(1ul << 0);
    while (DMA2_Stream7->CR & (1 << 0));

    // 2. Set the peripheral port register address in the DMA_SxPAR register.
    DMA2_Stream7->PAR = (uint32_t)(&(DCMI->DR));

    // 3. set the target memory addres in the M0AR / M1AR (if used) address.
    DMA2_Stream7->M0AR = (uint32_t)(camera_rawbuf[0]);
    DMA2_Stream7->M1AR = (uint32_t)(camera_rawbuf[1]);

    // 4. Configure the total number of data items to be transferred in the NDTR register.
    DMA2_Stream7->NDTR = ((uint16_t)(sizeof(camera_rawbuf[0]) / 4));

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


// This piece of dead code was used for testing the trigger. Keeping it cause I'll probably need to
// bring it back
#if 0
void footask(void* args)
{
    /*
      footask_handle = xTaskCreateStatic(footask,
                                       "foo task",
                                       FOO_TASK_BUFSZ,
                                       0,
                                       osPriorityNormal,
                                       FOOTaskBuffer,
                                       &FOOTaskControlBlock);
    */
    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        osDelay(10);
        HAL_GPIO_WritePin(hm01b0_trig_GPIO_Port, hm01b0_trig_Pin, GPIO_PIN_RESET);
    }
}

#define FOO_TASK_BUFSZ 512
osThreadId FOOTaskHandle;
uint32_t FOOTaskBuffer[ FOO_TASK_BUFSZ ];
osStaticThreadDef_t FOOTaskControlBlock;
#endif

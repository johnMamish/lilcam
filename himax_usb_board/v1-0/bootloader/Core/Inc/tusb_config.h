#ifndef _TUSB_CONFIG_H_
#define _TUSB_CONFIG_H_

//--------------------------------------------------------------------+
// Board Specific Configuration
//--------------------------------------------------------------------+

// Roothub Port number is 0 - we only have a single port on this MCU.
#define BOARD_TUD_RHPORT      0
#define TUD_OPT_RHPORT        0

#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE | OPT_MODE_HIGH_SPEED)
#define CFG_TUSB_RHPORT1_MODE 0

//
#define BOARD_TUD_MAX_SPEED OPT_MODE_HIGH_SPEED

//--------------------------------------------------------------------
// COMMON CONFIGURATION
//--------------------------------------------------------------------

// defined by board.mk
#define CFG_TUSB_MCU OPT_MCU_STM32F7

// user FreeRTOS
#define CFG_TUSB_OS OPT_OS_NONE

// turn off debug
#define CFG_TUSB_DEBUG 0

// Enable Device stack
#define CFG_TUD_ENABLED 1

#define CFG_TUD_MAX_SPEED OPT_MODE_HIGH_SPEED

/**
 * These macros allow us to specify memory region and alignment for USB endpoint buffers.
 */
#define CFG_TUSB_MEM_SECTION
#define CFG_TUSB_MEM_ALIGN __attribute__((aligned(16)))

//--------------------------------------------------------------------
// DEVICE CONFIGURATION
//--------------------------------------------------------------------

// endpoint 0 has default size of 64.
#define CFG_TUD_ENDPOINT0_SIZE 64

//---------------- Class ----------------
// how many interfaces of each class do we have?
#define CFG_TUD_DFU    1
#define CFG_TUD_DFU_XFER_BUFSIZE 512

#endif /* _TUSB_CONFIG_H_ */

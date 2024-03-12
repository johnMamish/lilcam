#ifndef _CAMERA_READ_TASK
#define _CAMERA_READ_TASK

#include "main.h"
#include "cmsis_os.h"

typedef enum camera_read_config_type {
    // specifies image sensor size for DCMI
    CAMERA_READ_CONFIG_SETSIZE,

    // specifies image sensor crop for DCMI
    CAMERA_READ_CONFIG_SETCROP,

    // Specifies whether or not to pack nibbles into bytes.
    // This option should be specified if switching to the hm01b0 on this board, which
    // sends bytes in 2 cycles
    CAMERA_READ_CONFIG_SETPACKING,

    // This command can stop or start DCMI reads.
    // DCMI reading must be stopped before crop or size are changed, but DCMI reads won't halt
    // until frame end is reached.
    // If a DCMI halt is pending, camera_read_task will defer further config requests until the
    // DCMI has been halted.
    // If no DCMI halt is pending but there are config requests, then the requests will be discarded
    CAMERA_READ_CONFIG_HALT
} camera_read_config_type_e;

/**
 * This preapres the camera read process to change...
 *  - image size
 *  - image crop
 *  - nibble packing
 *  - stop / start dcmi (so that cameras can be switched / restarted).
 */
typedef struct camera_read_config {
    camera_read_config_type_e config_type;

    union {
        //
        struct {
            int width;
            int height;
        } image_dims;

        // crop start and length
        struct {
            int start_x;
            int start_y;
            int len_x;
            int len_y;
        } crop_dims;

        // Should we pack nibbles into a byte before transmitting? true or false.
        struct {
            bool pack;
        } pack_options;

        //
        struct {
            void (*halt_callback)(void**);
            void** halt_callback_user;
            bool halt;
        } halt_options;
    } params;
} camera_read_config_t;

void camera_read_task(void const* args);

/**
 * Sends a request to change camera config.
 */
void camera_read_task_enqueue_config(const camera_read_config_t* req);

// These utility methods are utility methods for accessing camera_read_task_enqueue_config.
// They should only be called if the DCMI interface has been disabled.
void camera_read_task_set_size(int width, int height);
void camera_read_task_set_crop(int start_x, int start_y, int len_x, int len_y);
void camera_read_task_enable_packing();
void camera_read_task_disable_packing();

/**
 * Asks the camera read task to halt the DCMI interface. Blocks until this is done.
 */
void camera_read_task_halt_dcmi();

/**
 * Asks the camera read task to resume the DCMI interface. Blocks until this is done.
 */
void camera_read_task_resume_dcmi();

#endif

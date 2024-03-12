#ifndef _CAMERA_MANAGEMENT_TASK
#define _CAMERA_MANAGEMENT_TASK

#include "main.h"
#include "cmsis_os.h"
#include "hm01b0_init_bytes.h"

typedef enum camera_management_type {
    /**
     * Raw i2c write to a camera config register.
     * For some registers (like ones that affect image resolution), this might cause the camera to
     * crash. Changing image resolution should instead be done via the dedicated functions so that
     *
     * UNIMPLEMENTED
     */
    CAMERA_MANAGEMENT_TYPE_REG_WRITE,

    /**
     * Selects whether the hm01b0 or the hm0360 is active.
     * This command will have to stop the DCMI interface and reconfigure the DCMI.
     *
     * UNIMPLEMENTED
     */
    CAMERA_MANAGEMENT_TYPE_SENSOR_SEL,

    /**
     * This has a bad smell because it breaks a level of abstraction: the camera_management_task
     * will communicate with the
     *
     * However, it may be important for the DCMI to be halted before
     *
     * TODO: possibly deprecate this option.
     */
    CAMERA_MANAGEMENT_TYPE_DCMI_HALT,
    CAMERA_MANAGEMENT_TYPE_DCMI_RESUME,

    /**
     * Configures the frequency of the trigger and whether this camera is a controller or peripheral
     *
     * UNIMPLEMENTED
     */
    CAMERA_MANAGEMENT_TYPE_TRIGGER_CONFIG,
} camera_management_type_e;

enum {
    CAMERA_MANAGEMENT_SENSOR_SELECT_HM01B0 = 0,
    CAMERA_MANAGEMENT_SENSOR_SELECT_HM0360 = 1
};

typedef struct camera_management_request {
    camera_management_type_e request_type;

    union {
        hm01b0_reg_write_t reg_write;
        int sensor_select;
        struct {
            //
            int period;
            int controller;
        } trigger_setup;
    } params;
} camera_management_request_t;

/**
 * This task is responsible for configuring cameras
 *
 *  - setup
 *  - triggering
 *  - changing configs on the fly
 *
 */
void camera_management_task(void const* args);

/**
 *
 */
void camera_management_task_enqueue_request(const camera_management_request_t* req);

#endif

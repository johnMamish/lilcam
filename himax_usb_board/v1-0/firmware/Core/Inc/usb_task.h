#ifndef _USB_TASK_H
#define _USB_TASK_H

typedef struct usb_write_request {
    /// Source memory to copy from
    void* buf;

    /// length of buffer
    uint32_t len;
} usb_write_request_t;

void usb_task(void const* args);

#endif

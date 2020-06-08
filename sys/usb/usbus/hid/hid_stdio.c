
#define USB_H_USER_IS_RIOT_INTERNAL

#include "isrpipe.h"
#include "usb/usbus.h"
#include "usb/usbus/hid.h"

#if MODULE_VFS
#include "vfs.h"
#endif

#define ENABLE_DEBUG    (1)
#include "debug.h"

static usbus_hid_device_t hid;
static uint8_t _hid_tx_buf_mem[CONFIG_USBUS_HID_INTERRUPT_EP_SIZE];
static uint8_t _hid_rx_buf_mem[CONFIG_USBUS_HID_INTERRUPT_EP_SIZE];
static isrpipe_t _hid_stdio_isrpipe = ISRPIPE_INIT(_hid_rx_buf_mem);


void stdio_init(void)
{
}

ssize_t stdio_read(void* buffer, size_t size)
{
    return isrpipe_read(&_hid_stdio_isrpipe, buffer, size);
}

ssize_t stdio_write(const void* buffer, size_t size)
{
    const char *start = buffer;
    do {
        size_t n = usbus_hid_submit(&hid, buffer, size);
        usbus_hid_flush(&hid);

        buffer = (char*)buffer + n;
        size -= n;
    } while(size);

    return start - (char*)buffer;
}

static void _hid_rx_pipe(usbus_hid_device_t* hid, uint8_t *data, size_t len)
{
    (void)hid;
    for (size_t i = 0; i < len; i++) {
        isrpipe_write_one(&_hid_stdio_isrpipe, data[i]);
    }
}

void usb_hid_stdio_init(usbus_t* usbus, uint8_t* report_desc, size_t report_desc_size)
{
    usbus_hid_device_init(usbus, &hid, _hid_rx_pipe, _hid_tx_buf_mem,
                         sizeof(_hid_tx_buf_mem), report_desc, report_desc_size);
}
/*
 * Copyright (C) 2020 Nils Ollrogge
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup usbus_hid
 * @{
 * @file
 * @brief This file implements a USB HID callback and read/write functions.
 *
 * @author  Nils Ollrogge <nils-ollrogge@outlook.de>
 * @}
 */

#define USB_H_USER_IS_RIOT_INTERNAL

#include "isrpipe.h"
#include "isrpipe/read_timeout.h"

#include "usb/usbus.h"
#include "usb/usbus/hid.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

static usbus_hid_device_t hid;
static uint8_t _hid_tx_buf_mem[CONFIG_USBUS_HID_INTERRUPT_EP_SIZE];
static uint8_t _hid_rx_buf_mem[CONFIG_USBUS_HID_INTERRUPT_EP_SIZE];
static isrpipe_t _hid_stdio_isrpipe = ISRPIPE_INIT(_hid_rx_buf_mem);

int usb_hid_io_read(void *buffer, size_t size)
{
    return isrpipe_read(&_hid_stdio_isrpipe, buffer, size);
}

int usb_hid_io_read_timeout(void *buffer, size_t size, uint32_t timeout)
{
    return isrpipe_read_timeout(&_hid_stdio_isrpipe, buffer, size, timeout);
}

ssize_t usb_hid_io_write(const void *buffer, size_t size)
{
    const char *start = buffer;

    do {
        size_t n = usbus_hid_submit(&hid, buffer, size);
        usbus_hid_flush(&hid);

        buffer = (char *)buffer + n;
        size -= n;
    } while (size);

    return start - (char *)buffer;
}

static void _hid_rx_pipe(usbus_hid_device_t *hid, uint8_t *data, size_t len)
{
    (void)hid;
    for (size_t i = 0; i < len; i++) {
        isrpipe_write_one(&_hid_stdio_isrpipe, data[i]);
    }
}

void usb_hid_io_init(usbus_t *usbus, uint8_t *report_desc,
                     size_t report_desc_size)
{
    usbus_hid_device_init(usbus, &hid, _hid_rx_pipe, _hid_tx_buf_mem,
                          sizeof(_hid_tx_buf_mem), report_desc,
                          report_desc_size);
}

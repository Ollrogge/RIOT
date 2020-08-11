/*
 * Copyright (C) 2020 Nils Ollrogge
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

/**
 * @defgroup    usbus_hid USBUS HID
 * @ingroup     usb
 * @brief       USBUS HID interface module
 *
 * @{
 *
 * @file
 * @brief       Interface and definitions for USB HID type interfaces in
 *              USBUS.
 *
 *              The functionality provided here only implements the USB
 *              specific handling. A different module is required to provide
 *              functional handling of the data e.g. UART or STDIO integration.
 *
 * @author      Nils Ollrogge <nils-ollrogge@outlook.de>
 */

#ifndef USB_USBUS_HID_H
#define USB_USBUS_HID_H

#include <stdint.h>
#include "usb/usbus.h"
#include "usb/hid.h"
#include "tsrb.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief USB HID interrupt endpoint size
 */
#define CONFIG_USBUS_HID_INTERRUPT_EP_SIZE    0x40

/**
 * @brief USBUS HID context struct forward declaration
 */
typedef struct usbus_hid_device usbus_hid_device_t;

/**
 * @brief HID data callback.
 *
 * Callback for received data from the USB host
 *
 * @param[in]   hid     HID handler context
 * @param[in]   data    ptr to the data
 * @param[in]   len     Length of the received data
 */
typedef void (*usbus_hid_cb_t)(usbus_hid_device_t *hid, uint8_t *data,
                               size_t len);

/**
 * @brief USBUS HID context struct
 */
struct usbus_hid_device {
    usbus_handler_t handler_ctrl;   /**< control handler */
    usbus_interface_t iface;        /**< HID interface */
    usbus_descr_gen_t hid_descr;    /**< HID descriptor generator */
    uint8_t *report_desc;           /**< report descriptor reference */
    size_t report_desc_size;        /**< report descriptor size */
    usbus_t *usbus;                 /**< USBUS reference */
    tsrb_t tsrb;                    /**< TSRB for data to the host */
    size_t occupied;                /**< Number of bytes for the host */
    event_t flush;                  /**< device2host forced flush event  */
    usbus_hid_cb_t cb;              /**< Callback for data handlers */
};

/**
 * @brief Initialize an USBUS HID interface
 *
 * @param[in]   usbus               USBUS context to register with
 * @param[in]   hid                 USBUS HID handler
 * @param[in]   cb                  Callback for data from the USB interface
 * @param[in]   buf                 Buffer for data to the USB interface
 * @param[in]   len                 Size in bytes of the buffer
 * @param[in]   report_desc         USB_HID report descriptor
 * @param[in]   report_desc_size    Size of USB_HID report descriptor
 */
void usbus_hid_device_init(usbus_t *usbus, usbus_hid_device_t *hid,
                           usbus_hid_cb_t cb,
                           uint8_t *buf, size_t len, uint8_t *report_desc,
                           size_t report_desc_size);

/**
 * @brief Submit bytes to the HID handler
 *
 * @param[in]   hid         USBUS HID handler context
 * @param[in]   buf         buffer to submit
 * @param[in]   len         length of the submitted buffer
 *
 * @return                  Number of bytes added to the HID ring buffer
 */
size_t usbus_hid_submit(usbus_hid_device_t *hid, const uint8_t *buf,
                        size_t len);

/**
 * @brief Flush the buffer to the USB host
 *
 * @param[in]   hid      USBUS HID handler context
 */
void usbus_hid_flush(usbus_hid_device_t *hid);

#ifdef __cplusplus
}
#endif

#endif /* USB_USBUS_HID_H */
/** @} */

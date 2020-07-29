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
#include "tsrb.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief USB HID type descriptor
 */
#define USB_TYPE_DESCRIPTOR_HID      0x21

/**
 * @brief USB HID version in BCD
 */
#define USB_HID_VERSION_BCD          0x0110

/**
 * @name USB HID subclass types
 * @{
 */
#define USB_HID_SUBCLASS_NONE 0x0
#define USB_HID_SUBCLASS_BOOT 0x1
/** @} */

/**
 * @name USB HID protocol types
 * @{
 */
#define USB_HID_PROTOCOL_NONE       0x0
#define USB_HID_PROTOCOL_KEYBOARD   0x1
#define USB_HID_PROTOCOL_MOUSE      0x2
/** @} */

/**
 * @name USB HID descriptor types
 * @{
 */
#define USB_HID_DESCR_HID       0x21
#define USB_HID_DESCR_REPORT    0x22
#define USB_HID_DESCR_PHYSICAL  0x23
/** @} */

/**
 * @brief  USB HID country codes
 */
#define USB_HID_COUNTRY_CODE_NOTSUPPORTED 0x00

/**
 * @brief USB HID interrupt endpoint size
 */
#define CONFIG_USBUS_HID_INTERRUPT_EP_SIZE    0x40

/**
 * @name USB HID class specific control requests
 * @{
 */
#define USB_HID_REQUEST_GET_REPORT    0x01
#define USB_HID_REQUEST_GET_IDLE      0x02
#define USB_HID_REQUEST_GET_PROTOCOL  0x03
#define USB_HID_REQUEST_SET_REPORT    0x09
#define USB_HID_REQUEST_SET_IDLE      0x0a
#define USB_HID_REQUEST_SET_PROTOCOL  0x0b
/** @} */

/**
 * @brief USB HID descriptor struct
 *
 * @see USB HID 1.11 spec section 6.2.1
 */
typedef struct __attribute__((packed)){
    uint8_t length;         /**< Numeric expression that is the total size of the HID descriptor */
    uint8_t desc_type;      /**< Constant name specifying type of HID descriptor. */

    uint16_t bcd_hid;       /**< Numeric expression identifying the HID Class Specification release */
    uint8_t country_code;   /**< Numeric expression identifying country code of the localized hardware.  */
    uint8_t num_descrs;     /**< Numeric expression specifying the number of class descriptors */

    uint8_t report_type;    /**< Type of HID class report. */
    uint16_t report_length; /**< the total size of the Report descriptor. */
} usb_desc_hid_t;

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

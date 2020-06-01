
#ifndef USB_HID_H
#define USB_HID_H

#include <stdint.h>
#include "usb/usbus.h"
#include "tsrb.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USB_TYPE_DESCRIPTOR_HID      0x21 /**< USB HID type descriptor  */
#define USB_HID_VERSION_BCD          0x0110 /**< USB HID version in BCD */

typedef struct __attribute__((packed))
{
  uint8_t  length;         /**< Numeric expression that is the total size of the HID descriptor */
  uint8_t  desc_type;          /**< Constant name specifying type of HID descriptor. */

  uint16_t bcd_hid;          /**< Numeric expression identifying the HID Class Specification release */
  uint8_t  country_code;    /**< Numeric expression identifying country code of the localized hardware.  */
  uint8_t  num_descrs;     /**< Numeric expression specifying the number of class descriptors */

  uint8_t  report_type;     /**< Type of HID class report. */
  uint16_t report_length;   /**< the total size of the Report descriptor. */
} usb_desc_hid_t;

/**
 * @brief USBUS HID context struct forward declaration
 */
typedef struct usbus_hid_device usbus_hid_device_t;

void usbus_hid_device_init(usbus_t*, usbus_hid_device_t*);

typedef void (*usbus_hid_cb_t)(usbus_hid_device_t*, uint8_t*, size_t);

struct usbus_hid_device {
    usbus_handler_t handler_ctrl;
    usbus_interface_t iface;
    usbus_descr_gen_t hid_descr;
    usbus_hid_cb_t cb;
    tsrb_t tsrb;
    usbus_t *usbus;
};

/**
 * @name USB HID subclass types
 * @{
 */
#define USB_HID_SUBCLASS_NONE 0x0
#define USB_HID_SUBCLASS_BOOT 0x1

/**
 * @name USB HID protocol types
 * @{
 */
#define USB_HID_PROTOCOL_NONE 0x0 /**< None */
#define USB_HID_PROTOCOL_KEYBOARD  0x1
#define USB_HID_PROTOCOL_MOUSE  0x2

/**
 * @name USB HID descriptor types
 * @{
 */
#define USB_HID_DESCR_HID  0x21
#define USB_HID_DESCR_REPORT 0x22
#define USB_HID_DESCR_PHYSICAL 0x23

/**
 * @name USB HID country codes
 * @{
 */

#define USB_HID_COUNTRY_CODE_NOTSUPPORTED 0x00


/**
 * @brief USB HID bulk endpoint size
 */
#define CONFIG_USBUS_HID_INTERRUPT_EP_SIZE    0x40


#define USB_HID_REQUEST_GET_REPORT 0x01
#define USB_HID_REQUEST_GET_IDLE 0x02
#define USB_HID_REQUEST_GET_PROTOCOL 0x03
#define USB_HID_REQUEST_SET_REPORT 0x09
#define USB_HID_REQUEST_SET_IDLE 0x0a
#define USB_HID_REQUEST_SET_PROTOCOL 0x0b


#define USBUS_HID_LINE_STATE_IDLE 0x00
#define USBUS_HID_LINE_STATE_BUSY 0x01

#ifdef __cplusplus
}
#endif

#endif
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
 *
 * @author  Nils Ollrogge <nils-ollrogge@outlook.de>
 * @}
 */

#define USB_H_USER_IS_RIOT_INTERNAL

#include <string.h>

#include "usb/usbus.h"
#include "usb/usbus/control.h"
#include "usb/usbus/hid.h"
#include "tsrb.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

static void _init(usbus_t *usbus, usbus_handler_t *handler);
static void _event_handler(usbus_t *usbus, usbus_handler_t *handler,
                           usbus_event_usb_t event);
static int _control_handler(usbus_t *usbus, usbus_handler_t *handler,
                            usbus_control_request_state_t state,
                            usb_setup_t *setup);
static void _transfer_handler(usbus_t *usbus, usbus_handler_t *handler,
                              usbdev_ep_t *ep, usbus_event_transfer_t event);

static void _handle_flush(event_t *ev);
static void _handle_in(usbus_hid_device_t *hid, usbdev_ep_t *ep);

static const usbus_handler_driver_t hid_driver = {
    .init = _init,
    .event_handler = _event_handler,
    .control_handler = _control_handler,
    .transfer_handler = _transfer_handler
};

static size_t _gen_hid_descriptor(usbus_t *usbus, void *arg);

static const usbus_descr_gen_funcs_t _hid_descriptor = {
    .fmt_post_descriptor = _gen_hid_descriptor,
    .len = {
        .fixed_len = sizeof(usb_desc_hid_t)
    },
    .len_type = USBUS_DESCR_LEN_FIXED
};

static size_t _gen_hid_descriptor(usbus_t *usbus, void *arg)
{
    usbus_hid_device_t *hid_dev = arg;
    usb_desc_hid_t hid_desc;

    hid_desc.length = sizeof(usb_desc_hid_t);
    hid_desc.desc_type = USB_HID_DESCR_HID;
    hid_desc.bcd_hid = USB_HID_VERSION_BCD;
    hid_desc.country_code = USB_HID_COUNTRY_CODE_NOTSUPPORTED;
    hid_desc.num_descrs = 0x01;
    hid_desc.report_type = USB_HID_DESCR_REPORT;
    hid_desc.report_length = hid_dev->report_desc_size;

    usbus_control_slicer_put_bytes(usbus, (uint8_t *)&hid_desc,
                                   sizeof(hid_desc));
    return sizeof(usb_desc_hid_t);
}

size_t usbus_hid_submit(usbus_hid_device_t* hid, const uint8_t *buf, size_t len)
{
    size_t n;
    unsigned int old;

    old = irq_disable();
    n = tsrb_add(&hid->tsrb, buf, len);
    irq_restore(old);

    return n;
}

void usbus_hid_flush(usbus_hid_device_t *hid)
{
    if (hid->usbus) {
        usbus_event_post(hid->usbus, &hid->flush);
    }
}

static void _handle_flush(event_t *ev)
{
    usbus_hid_device_t *hid = container_of(ev, usbus_hid_device_t, flush);

    if (hid->occupied == 0) {
        _handle_in(hid, hid->iface.ep->next->ep);
    }
}

void usbus_hid_device_init(usbus_t *usbus, usbus_hid_device_t *hid,
                           usbus_hid_cb_t cb,
                           uint8_t *buf, size_t len, uint8_t *report_desc,
                           size_t report_desc_size)
{
    memset(hid, 0, sizeof(usbus_hid_device_t));
    hid->usbus = usbus;
    tsrb_init(&hid->tsrb, buf, len);
    hid->handler_ctrl.driver = &hid_driver;
    hid->report_desc = report_desc;
    hid->report_desc_size = report_desc_size;
    hid->cb = cb;

    DEBUG("hid_init: %d %d \n", report_desc_size, report_desc[0]);
    usbus_register_event_handler(usbus, &hid->handler_ctrl);
}

static void _init(usbus_t *usbus, usbus_handler_t *handler)
{
    DEBUG("USB_HID: initialization\n");
    usbus_hid_device_t *hid = (usbus_hid_device_t *)handler;

    hid->flush.handler = _handle_flush;

    hid->hid_descr.next = NULL;
    hid->hid_descr.funcs = &_hid_descriptor;
    hid->hid_descr.arg = hid;

    /*
    Configure Interface as USB_HID interface, choosing NONE for subclass and
    protocol in order to represent a generic I/O device
    */
    hid->iface.class = USB_CLASS_HID;
    hid->iface.subclass = USB_HID_SUBCLASS_NONE;
    hid->iface.protocol = USB_HID_PROTOCOL_NONE;
    hid->iface.descr_gen = &hid->hid_descr;
    hid->iface.handler = handler;

    usbus_endpoint_t *ep = usbus_add_endpoint(usbus, &hid->iface,
                                              USB_EP_TYPE_INTERRUPT,
                                              USB_EP_DIR_IN,
                                              CONFIG_USBUS_HID_INTERRUPT_EP_SIZE);

    /* interrupt endpoint polling rate */
    ep->interval = 0x05;
    usbus_enable_endpoint(ep);

    ep = usbus_add_endpoint(usbus, &hid->iface,
                            USB_EP_TYPE_INTERRUPT, USB_EP_DIR_OUT,
                            CONFIG_USBUS_HID_INTERRUPT_EP_SIZE);


    /* interrupt endpoint polling rate */
    ep->interval = 0x05;
    usbus_enable_endpoint(ep);

    /* signal that INTERRUPT OUT ready to receive data */
    usbdev_ep_ready(ep->ep, 0);

    usbus_add_interface(usbus, &hid->iface);
}

static void _event_handler(usbus_t *usbus, usbus_handler_t *handler,
                           usbus_event_usb_t event)
{
    (void)usbus;
    (void)handler;

    switch (event) {
        default:
            DEBUG("USB HID unhandeled event: 0x%x\n", event);
            break;
    }
}

static int _control_handler(usbus_t *usbus, usbus_handler_t *handler,
                            usbus_control_request_state_t state,
                            usb_setup_t *setup)
{
    usbus_hid_device_t *hid = (usbus_hid_device_t *)handler;

    DEBUG("USB_HID: request: %d type: %d value: %d length: %d state: %d \n",
          setup->request, setup->type, setup->value >> 8, setup->length, state);

    /* Requests defined in USB HID 1.11 spec section 7 */
    switch (setup->request) {
        case USB_SETUP_REQ_GET_DESCRIPTOR: {
            uint8_t desc_type = setup->value >> 8;
            if (desc_type == USB_HID_DESCR_REPORT) {
                usbus_control_slicer_put_bytes(usbus, hid->report_desc,
                                               hid->report_desc_size);
            }
            else if (setup->value >> 8 == USB_HID_DESCR_HID) {
                _gen_hid_descriptor(usbus, NULL);
            }
            break;
        }
        case USB_HID_REQUEST_GET_REPORT:
            break;
        case USB_HID_REQUEST_GET_IDLE:
            break;
        case USB_HID_REQUEST_GET_PROTOCOL:
            break;
        case USB_HID_REQUEST_SET_REPORT:
            if ((state == USBUS_CONTROL_REQUEST_STATE_OUTDATA)) {
                size_t size = 0;
                uint8_t *data = usbus_control_get_out_data(usbus, &size);
                if (size > 0) {
                    hid->cb(hid, data, size);
                }
            }
            break;
        case USB_HID_REQUEST_SET_IDLE:
            break;
        case USB_HID_REQUEST_SET_PROTOCOL:
            break;
        default:
            DEBUG("USB_HID: unknown request %d \n", setup->request);
            return -1;
    }
    return 1;
}

static void _handle_in(usbus_hid_device_t *hid, usbdev_ep_t *ep)
{
    if (hid->usbus->state != USBUS_STATE_CONFIGURED) {
        return;
    }

    unsigned int old = irq_disable();

    while (!tsrb_empty(&hid->tsrb)) {
        int c = tsrb_get_one(&hid->tsrb);
        ep->buf[hid->occupied++] = (uint8_t)c;
        if (hid->occupied >= CONFIG_USBUS_HID_INTERRUPT_EP_SIZE) {
            break;
        }
    }

    irq_restore(old);

    DEBUG("USB_HID _handle_in %d \n ", hid->occupied);
    usbdev_ep_ready(ep, hid->occupied);
}

static void _transfer_handler(usbus_t *usbus, usbus_handler_t *handler,
                              usbdev_ep_t *ep, usbus_event_transfer_t event)
{
    (void)usbus;
    (void)event;
    DEBUG("USB_HID: transfer_handler\n");

    usbus_hid_device_t *hid = (usbus_hid_device_t *)handler;

    if ((ep->dir == USB_EP_DIR_IN) && (ep->type == USB_EP_TYPE_INTERRUPT)) {
        hid->occupied = 0;
        if (!tsrb_empty(&hid->tsrb)) {
            return _handle_in(hid, ep);
        }
    }
    else if ((ep->dir == USB_EP_DIR_OUT) &&
             (ep->type == USB_EP_TYPE_INTERRUPT)) {
        size_t len;
        usbdev_ep_get(ep, USBOPT_EP_AVAILABLE, &len, sizeof(size_t));
        if (len > 0) {
            hid->cb(hid, ep->buf, len);
        }
        usbdev_ep_ready(ep, 0);
    }
}

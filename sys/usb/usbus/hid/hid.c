

#define USB_H_USER_IS_RIOT_INTERNAL

#include <string.h>

#include "usb/usbus.h"
#include "usb/usbus/control.h"
#include "usb/usbus/hid.h"
#include "tsrb.h"

#define ENABLE_DEBUG    (1)
#include "debug.h"

static void _init(usbus_t *usbus, usbus_handler_t *handler);
static void _event_handler(usbus_t *usbus, usbus_handler_t *handler, usbus_event_usb_t event);
static int _control_handler(usbus_t *usbus, usbus_handler_t *handler,
                            usbus_control_request_state_t state,
                            usb_setup_t *setup);
static void _transfer_handler(usbus_t *usbus, usbus_handler_t *handler,
                             usbdev_ep_t *ep, usbus_event_transfer_t event);


static uint8_t report_desc_ctap[] = {
  0x06, 0xD0, 0xF1, // HID_UsagePage ( FIDO_USAGE_PAGE ),
  0x09, 0x01, // HID_Usage ( FIDO_USAGE_CTAPHID ),
  0xA1, 0x01, // HID_Collection ( HID_Application ),
  0x09, 0x20, // HID_Usage ( FIDO_USAGE_DATA_IN ),
  0x15, 0x00, // HID_LogicalMin ( 0 ),
  0x26, 0xFF, 0x00, // HID_LogicalMaxS ( 0xff ),
  0x75, 0x08, // HID_ReportSize ( 8 ),
  0x95, 0x40, // HID_ReportCount ( HID_INPUT_REPORT_BYTES ),
  0x81, 0x02, // HID_Input ( HID_Data | HID_Absolute | HID_Variable ),
  0x09, 0x21, // HID_Usage ( FIDO_USAGE_DATA_OUT ),
  0x15, 0x00, // HID_LogicalMin ( 0 ),
  0x26, 0xFF, 0x00, // HID_LogicalMaxS ( 0xff ),
  0x75, 0x08, // HID_ReportSize ( 8 ),
  0x95, 0x40, // HID_ReportCount ( HID_OUTPUT_REPORT_BYTES ),
  0x91, 0x02, // HID_Output ( HID_Data | HID_Absolute | HID_Variable ),
  0xC0, // HID_EndCollection
};

static const usbus_handler_driver_t hid_driver = {
    .init = _init,
    .event_handler = _event_handler,
    .control_handler = _control_handler,
    .transfer_handler = _transfer_handler
};

static size_t _gen_hid_descriptor(usbus_t *usbus, void* arg);
//static size_t _gen_report_desc_ctap(usbus_t *usbus, void* arg);

static const usbus_descr_gen_funcs_t _hid_descriptor = {
    .fmt_post_descriptor = _gen_hid_descriptor,
    .len = {
        .fixed_len = sizeof(usb_desc_hid_t)
    },
    .len_type = USBUS_DESCR_LEN_FIXED
};

static size_t _gen_hid_descriptor(usbus_t *usbus, void* arg)
{
    (void)arg;

    usb_desc_hid_t hid;

    hid.length = sizeof(usb_desc_hid_t);
    hid.desc_type = USB_HID_DESCR_HID;
    hid.bcd_hid = USB_HID_VERSION_BCD;
    hid.country_code = USB_HID_COUNTRY_CODE_NOTSUPPORTED;
    hid.num_descrs = 0x01;
    hid.report_type = USB_HID_DESCR_REPORT;
    hid.report_length = sizeof(report_desc_ctap);

    DEBUG("SIZE: %d %d \n", sizeof(hid), sizeof(report_desc_ctap));

    usbus_control_slicer_put_bytes(usbus, (uint8_t*)&hid, sizeof(hid));
    return sizeof(usb_desc_hid_t);
}

/*
static size_t _gen_report_desc_ctap(usbus_t *usbus, void* arg) 
{
    (void)arg;
    (void)usbus;

    usbus_control_slicer_put_bytes(usbus, report_desc_ctap, sizeof(report_desc_ctap));
    return sizeof(report_desc_ctap);
}
*/

void usbus_hid_device_init(usbus_t *usbus, usbus_hid_device_t *hid)
{
    DEBUG("USB_HID: device_init\n");
    memset(hid, 0, sizeof(usbus_hid_device_t));
    hid->usbus = usbus;
    hid->handler_ctrl.driver = &hid_driver;
    usbus_register_event_handler(usbus, &hid->handler_ctrl);
}

static void _init(usbus_t *usbus, usbus_handler_t *handler) 
{
    DEBUG("USB_HID: initialization\n");
    usbus_hid_device_t *hid = (usbus_hid_device_t*)handler;

    hid->hid_descr.next = NULL;
    hid->hid_descr.funcs = &_hid_descriptor;
    hid->hid_descr.arg = hid;

    hid->iface.class = USB_CLASS_HID;
    hid->iface.subclass = USB_HID_SUBCLASS_NONE;
    hid->iface.protocol = USB_HID_PROTOCOL_NONE;
    hid->iface.descr_gen = &hid->hid_descr;
    hid->iface.handler = handler;

    usbus_endpoint_t *ep = usbus_add_endpoint(usbus, &hid->iface, 
                                              USB_EP_TYPE_INTERRUPT, USB_EP_DIR_IN, 
                                              CONFIG_USBUS_HID_INTERRUPT_EP_SIZE);
    
    ep->interval = 0x05;
    usbus_enable_endpoint(ep);
    
    ep = usbus_add_endpoint(usbus, &hid->iface,  
                            USB_EP_TYPE_INTERRUPT, USB_EP_DIR_OUT,
                            CONFIG_USBUS_HID_INTERRUPT_EP_SIZE);

    ep->interval = 0x05;
    usbus_enable_endpoint(ep);

    usbus_add_interface(usbus, &hid->iface);
}

static void _handle_reset(usbus_handler_t *handler)
{
    usbus_hid_device_t *hid = (usbus_hid_device_t*)handler;
    DEBUG("USB HID: Reset notification received\n");
    (void)hid;

}

static void _event_handler(usbus_t *usbus, usbus_handler_t *handler, usbus_event_usb_t event)
{
    (void)usbus;
    switch(event) {
        case USBUS_EVENT_USB_RESET:
            _handle_reset(handler);
            break;
        default:
            DEBUG("USB HID unhandeled event: 0x%x\n", event);
            break;
    }
}

static int _control_handler(usbus_t *usbus, usbus_handler_t *handler,
                            usbus_control_request_state_t state,
                            usb_setup_t *setup)
{
    (void)usbus;
    (void)handler;
    (void)state;
    (void)setup;
    //usbus_hid_device_t *hid = (usbus_hid_device_t*)handler;
    DEBUG("TYPE: %d %d \n ", setup->value >> 8, setup->value & 0x00FF)
    switch(setup->request) {
        case USB_HID_REQUEST_GET_REPORT:
            DEBUG("USB_HID: 1 \n");
            break;
        case USB_HID_REQUEST_GET_IDLE:
            DEBUG("USB_HID: 2 \n");
            break;
        case USB_HID_REQUEST_GET_PROTOCOL:
            DEBUG("USB_HID: 3 \n");
            break;
        case USB_HID_REQUEST_SET_REPORT:
            DEBUG("USB_HID: 4 \n");
            break;
        case USB_HID_REQUEST_SET_IDLE:
            DEBUG("USB_HID: 5 \n");
            break;
        case USB_HID_REQUEST_SET_PROTOCOL:
            DEBUG("USB_HID: 6 \n");
            break;
        default:
            DEBUG("USB_HID: unknown request %x \n", setup->request);
            break;
    }

    return 1;
}

static void _transfer_handler(usbus_t *usbus, usbus_handler_t *handler,
                             usbdev_ep_t *ep, usbus_event_transfer_t event)
{
    (void)usbus;
    (void)handler;
    (void)ep;
    (void)event;
    DEBUG("USB_HID: transfer_handler\n");
}


                            
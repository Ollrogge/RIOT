#include <stdio.h>
#include <stdlib.h>

#include "usb/usbus.h"
#include "xtimer.h"
#include "usb/usbus/hid.h"

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

static usbus_t usbus;
static char _stack[USBUS_STACKSIZE];

void usb_hid_stdio_init(usbus_t* usbus, uint8_t* report_desc, size_t report_desc_size);
ssize_t usb_hid_stdio_read(void* buffer, size_t len);

void init(void)
{
  usbdev_t *usbdev = usbdev_get_ctx(0);
  usbus_init(&usbus, usbdev);

  usb_hid_stdio_init(&usbus, report_desc_ctap, sizeof(report_desc_ctap));

  usbus_create(_stack, USBUS_STACKSIZE, USBUS_PRIO, USBUS_TNAME, &usbus);
}

int main(void)
{
  xtimer_sleep(3);
  init();
  puts("RIOT USB HID echo test");
  puts("Execute command 'dmesg' to get endpoint number (/dev/hidrawX)");
  puts("write to /dev/hidrawX to test echo output");
  puts("e.g echo 'Test' > /dev/hidraw6");

  uint8_t buffer[CONFIG_USBUS_HID_INTERRUPT_EP_SIZE];
  for (;;) {
    ssize_t len = usb_hid_stdio_read(buffer, CONFIG_USBUS_HID_INTERRUPT_EP_SIZE);

    printf("Msg received via USB HID: ");
    for (int i = 0; i < len; i++)  {
      putc(buffer[i], stdout);
    }
  }
}
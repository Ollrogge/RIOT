#ifdef CONFIG_LORAWAN_OVER_USB
#include "usb/usbus.h"
#include "usb/usbus/hid.h"
#include "usb/usbus/hid_io.h"
#endif
#include "net/gnrc/lorawan.h"

#define ENABLE_DEBUG 1
#include "debug.h"

static void _usb_cb(void* arg)
{
    (void)arg;
    // todo: size is guessed
    uint8_t buffer[0x100] = {0};
    int total_cnt = 0x0;
    int cnt = 0x0;

    do {
        cnt = usb_hid_io_read(buffer, CONFIG_USBUS_HID_INTERRUPT_EP_SIZE);
        total_cnt += cnt;
    } while(cnt == CONFIG_USBUS_HID_INTERRUPT_EP_SIZE && total_cnt != sizeof(buffer));
}

void gnrc_lorawan_usb_init(gnrc_lorawan_t *mac)
{
    (void)mac;
#if IS_ACTIVE(CONFIG_LORAWAN_OVER_USB)
    if (mac->usb_is_initialized) {
        return;
    }

    usb_hid_io_set_rx_cb(_usb_cb, NULL);
    mac->usb_is_initialized = true;
#endif
}

void gnrc_lorawan_usb_send(iolist_t *iolist)
{
    (void) iolist;
#if IS_ACTIVE(CONFIG_LORAWAN_OVER_USB)
    for (iolist_t *iol = iolist; iol; iol = iol->iol_next) {
       if (iol->iol_len > 0) {
           /* write data to payload buffer */
           usb_hid_io_write(iol->iol_base, iol->iol_len);
       }
    }
#endif
}
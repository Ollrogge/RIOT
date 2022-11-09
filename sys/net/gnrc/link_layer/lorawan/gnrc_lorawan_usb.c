#ifdef CONFIG_LORAWAN_OVER_USB
#include "usb/usbus.h"
#include "usb/usbus/hid.h"
#include "usb/usbus/hid_io.h"
#endif
#include "net/gnrc/lorawan.h"
#include "xtimer.h"
#include "cond.h"
#include "event.h"

#define ENABLE_DEBUG 1
#include "debug.h"

static cond_t _cond = COND_INIT;
static mutex_t _lock = MUTEX_INIT;
uint8_t recv_buf[0x100];
unsigned recv_off;
static void _usb_cb(void* arg)
{
    gnrc_lorawan_t *mac = (gnrc_lorawan_t*)arg;
    int cnt = 0x0;

    cnt = usb_hid_io_read(&recv_buf[recv_off], CONFIG_USBUS_HID_INTERRUPT_EP_SIZE);
    recv_off += cnt;

    if (cnt < CONFIG_USBUS_HID_INTERRUPT_EP_SIZE)  {
        cond_signal(&_cond);
    }
}

void gnrc_lorawan_usb_init(gnrc_lorawan_t *mac)
{
    (void)mac;
#if IS_ACTIVE(CONFIG_LORAWAN_OVER_USB)
    if (mac->usb_is_initialized) {
        return;
    }

    usb_hid_io_set_rx_cb(_usb_cb, mac);
    mac->usb_is_initialized = true;
#endif
}

void gnrc_lorawan_usb_send(gnrc_lorawan_t *mac, iolist_t *iolist)
{
    (void) iolist;
#if IS_ACTIVE(CONFIG_LORAWAN_OVER_USB)
    for (iolist_t *iol = iolist; iol; iol = iol->iol_next) {
       if (iol->iol_len > 0) {
           /* write data to payload buffer */
           usb_hid_io_write(iol->iol_base, iol->iol_len);
       }
    }

    mutex_lock(&_lock);
    cond_wait(&_cond, &_lock);
    mutex_unlock(&_lock);
    unsigned tmp = recv_off;
    recv_off = 0x0;
    gnrc_lorawan_mlme_process_join(mac, recv_buf, tmp);
#endif
}
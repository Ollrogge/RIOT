#include <string.h>
#include "fido2/ctap.h"
#include "iolist.h"

#define ENABLE_DEBUG      1
#include "debug.h"

iolist_t _data;
uint8_t buf[0x1 + sizeof(ctap_resp_t)];

iolist_t *gnrc_lorawan_fido_join_req1(void)
{
    //set byte 0 to indicate type of request (begin)
    buf[0] = 0x0;

    _data.iol_next = NULL;
    _data.iol_base = buf;
    _data.iol_len = 0x1;

    return &_data;
}
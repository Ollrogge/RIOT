/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup fido2_ctap
 * @{
 * @file
 *
 * @author  Nils Ollrogge <nils.ollrogge@fu-berlin.de>
 * @}
 */

#include "apdu.h"

int apdu_decode(uint8_t* data, size_t len, command_apdu_t* apdu)
{
    command_apdu_t* p = (command_apdu_t*)data;
    size_t off = 0;

    apdu->cla = p->cla;
    apdu->ins = p->ins;
    apdu->p1 = p->p1;
    apdu->p2 = p->p2;

    uint8_t b1 = p->le;

    /* case 1 */
    if (len == 4) {
        return 0;
    }

    /* case 2S */
    if (len == 5) {
        apdu->le = b1;
    }

    if (len == 1 + b1 && b1 != 0) {

    }


    return 0;
}
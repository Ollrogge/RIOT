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
    command_apdu_t* header = (command_apdu_header_t*)data;
    size_t header_len = sizeof(command_apdu_header_t);
    bool is_extended = false;

    memset(apdu, 0, sizeof(command_apdu_t));

    apdu->cla = header->cla;
    apdu->ins = header->ins;
    apdu->p1 = header->p1;
    apdu->p2 = header->p2;

    uint8_t b1 = data[header_len];

    /* case 1, L = 0 */
    if (len == header_len) {
        return 0;
    }
    /* case 2S, L = 1 */
    else if (len == header_len + 1) {
        apdu->le = b1;
        if (!apdu->le) {
            apdu->le = 0x100;
        }
    }
    /* Case 3S, L = 1 + (b1) */
    else if (len == header_len + 1 + b1 && b1 != 0) {
        apdu->lc = b1;
    }
    /* case 4S, L = 2 + (b1) */
    else if (len == header_len + 2 + b1 && b1 != 0) {
        apdu->lc = b1;
        apdu->le = data[len - 1];
        if (!apdu->le) {
            apdu->le = 0x100;
        }
    }
    /* extended cases */
    else if (len >= header_len + 3 && b1 == 0) {
        is_extended = true;

        /* (b2 || b3) */
        uint16_t encoded_len = (data[header_len + 2] << 8) + data[header_len + 3];

        /* invalid length */
        if (encoded_len > len - header_len - 3) {
            return -1;
        }
        /* case 2E, L = 3 + (b2 || b3) */
        if (len == header_len + 3) {
            apdu->le = encoded_len;
            if (!apdu->le) {
                apdu->le = 0x10000;
            }
        }
        /* case 3E */
        else if (len == header_len + 3 + encoded_len) {
            apdu->lc = encoded_len;
        }
        /* case 4E */
        else if (len == header_len + 5 + encoded_len) {
            apdu->lc = encoded_len;
            apdu->le = (data[len - 2] << 8) + data[len - 1];

            if (!apdu->le) {
                apdu->le = 0x10000;
            }
        }
        /* invalid length */
        else {
            return -1;
        }
    }
    /* invalid length */
    else {
        return -1;
    }

    if (apdu->lc) {
        if (is_extended) {
            apdu->data = data + header_len + 3;
        }
        else {
            apdu->data = data + header_len + 1;
        }
    }

    return 0;
}
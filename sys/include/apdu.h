/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 *
 * @{
 *
 * @file
 * @brief       Definitions for CTAP flash memory helper functions
 *
 * @author      Nils Ollrogge <nils.ollrogge@fu-berlin.de>
 */

#ifndef APDU_H
#define APDU_H

#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed))
{
    uint8_t cla;
    uint8_t ins;
    uint16_t p1;
    uint16_t p2;
    uint32_t lc;
    uint8_t* data;
    uint32_t le;
} command_apdu_t;

typedef struct __attribute__((packed))
{
    uint8_t* data;
    uint16_t sw1;
    uint16_t sw2;
} response_apdu_t;

int apdu_decode(uint8_t* data, size_t len, command_apdu_t* apdu);

#ifdef __cplusplus
}
#endif
#endif /* APDU_H */
/** @} */
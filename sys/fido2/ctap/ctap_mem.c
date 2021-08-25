/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup fido2_ctap_mem
 * @{
 * @file
 *
 * @author      Nils Ollrogge <nils.ollrogge@fu-berlin.de>
 * @}
 */

#include <string.h>

#include "mtd.h"
#include "mtd_flashpage.h"

#include "fido2/ctap/ctap_mem.h"

#define ENABLE_DEBUG    (0)
#include "debug.h"

/**
 * @brief   MTD device descriptor initalized with flash-page driver
 */
static mtd_dev_t _mtd_dev = MTD_FLASHPAGE_INIT_VAL(8);

/**
 * @brief   Max amount of resident keys that can be stored
 */
static uint16_t _max_rk_amnt;

int fido2_ctap_mem_init(void)
{
    int ret;

    ret = mtd_init(&_mtd_dev);

    if (ret < 0) {
        return ret;
    }

    for (unsigned i = CTAP_FLASH_RK_START_PAGE; i <= FLASHPAGE_NUMOF; i++) {
        _max_rk_amnt += flashpage_size(i);
    }

    return CTAP2_OK;
}

int fido2_ctap_mem_read(void *buf, uint32_t page, uint32_t offset, uint32_t size)
{
    assert(buf);

    if (mtd_read_page(&_mtd_dev, buf, page, offset, size) < 0) {
        return CTAP1_ERR_OTHER;
    }

    return CTAP2_OK;
}

int fido2_ctap_mem_write(const void *buf, uint32_t page, uint32_t offset, uint32_t size)
{
    assert(buf);

    if (mtd_write_page(&_mtd_dev, buf, page, offset, size) < 0) {
        return CTAP1_ERR_OTHER;
    }

    return CTAP2_OK;
}

uint16_t fido2_ctap_mem_get_max_rk_amount(void)
{
    return _max_rk_amnt;
}

int fido2_ctap_mem_get_flashpage_number_of_rk(uint16_t rk_idx)
{
    uint16_t idx = 0;

    for (unsigned i = CTAP_FLASH_RK_START_PAGE; i <= FLASHPAGE_NUMOF; i++) {
        idx += flashpage_size(i) / CTAP_FLASH_RK_SZ;

        if (idx >= rk_idx) {
            return i;
        }
    }

    return -1;
}

int fido2_ctap_mem_get_offset_of_rk_into_flashpage(uint16_t rk_idx)
{
    uint16_t idx = 0;
    uint16_t old_idx = 0;

    for (unsigned i = CTAP_FLASH_RK_START_PAGE; i <= FLASHPAGE_NUMOF; i++) {
        old_idx = idx;
        idx += flashpage_size(i) / CTAP_FLASH_RK_SZ;

        if (idx >= rk_idx) {
            return CTAP_FLASH_RK_SZ * (rk_idx - old_idx);
        }
    }

    return -1;
}
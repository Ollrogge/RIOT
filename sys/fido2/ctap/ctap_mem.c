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

#include "xtimer.h"
#define ENABLE_DEBUG    (0)
#include "debug.h"

#include "fido2/ctap/ctap_mem.h"

/**
 * @brief Write data to flash at an @param offset from the beginning of the flashpage
 */
static void _flash_write_raw(int page, int offset,
                             const void *data, size_t len);

/**
 * @brief Erase @param page and write data to it
 */
static void _flash_write(int page, const void *data, size_t len);

/**
 * @brief Verify that write to @param page at @param offset has been successful
 */
static int _flash_verify(int page, int offset, const void *data, size_t len);

/**
 * @brief Check that flash memory with @param len at @param offset from @param page is erased
 */
static bool _flash_is_erased(int page, int offset, size_t len);

void fido2_ctap_mem_read(int page, void *buf)
{
    assert(buf);
    return flashpage_read(page, buf);
}

/**
 * write_raw should fail once for every page => erase page
 * afterwards we should be able to selectively write to page with write_raw,
 * which is faster than erasing and writing whole pages.
 */
int fido2_ctap_mem_write_and_verify(int page, int offset,
                                    const void *data, size_t len)
{
    assert(data);
    assert(!(len % FLASHPAGE_WRITE_BLOCK_SIZE));
    assert(page < (int)FLASHPAGE_NUMOF);

    if (_flash_is_erased(page, offset, len)) {
        _flash_write_raw(page, offset, data, len);
    }
    else {
        _flash_write(page, data, len);
    }

    return _flash_verify(page, offset, data, len);
}

static bool _flash_is_erased(int page, int offset, size_t len)
{
    uint32_t *addr = (uint32_t *)((uint8_t *)flashpage_addr(page) + offset);

    for (size_t i = 0; i < len / 4; i++) {
        if (addr[i] != 0xffffffff) {
            return false;
        }
    }

    return true;
}

static void _flash_write_raw(int page, int offset,
                             const void *data, size_t len)
{
    uint32_t *addr = (uint32_t *)((uint8_t *)flashpage_addr(page) + offset);

    flashpage_write(addr, data, len);
}

static int _flash_verify(int page, int offset, const void *data, size_t len)
{
    uint8_t *addr = (uint8_t *)flashpage_addr(page) + offset;

    if (memcmp(addr, data, len) == 0) {
        return CTAP2_OK;
    }
    else {
        return CTAP1_ERR_OTHER;
    }
}

static void _flash_write(int page, const void *data, size_t len)
{
    uint32_t *page_addr = (uint32_t *)flashpage_addr(page);

    flashpage_erase(page);

    /* write data to page */
    if (data != NULL) {
        flashpage_write(page_addr, data, len);
    }
}

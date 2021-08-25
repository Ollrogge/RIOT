/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup    fido2_ctap_mem FIDO2 CTAP flash
 * @ingroup     fido2_ctap
 * @brief       FIDO2 CTAP flash memory helper
 *
 * @{
 *
 * @file
 * @brief       Definitions for CTAP flash memory helper functions
 *
 * @author      Nils Ollrogge <nils.ollrogge@fu-berlin.de>
 */

#ifndef FIDO2_CTAP_CTAP_MEM_H
#define FIDO2_CTAP_CTAP_MEM_H

#include <stdint.h>

#include "fido2/ctap/ctap.h"
#include "periph/flashpage.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Start page for storing resident keys
 */
#define CTAP_FLASH_RK_START_PAGE 15U

/**
 * @brief Page for storing authenticator state information
 */
#define CTAP_FLASH_STATE_PAGE CTAP_FLASH_RK_START_PAGE - 1

/**
 * @brief Calculate padding needed to align struct size for saving to flash
 */
#define CTAP_FLASH_ALIGN_PAD(x) (sizeof(x) % FLASHPAGE_WRITE_BLOCK_ALIGNMENT == \
                                 0 ? \
                                 0 : FLASHPAGE_WRITE_BLOCK_ALIGNMENT - \
                                 sizeof(x) % FLASHPAGE_WRITE_BLOCK_ALIGNMENT)

/**
 * @brief Resident key size with alignment padding
 */
#define CTAP_FLASH_RK_SZ (sizeof(struct ctap_resident_key) + \
                          CTAP_FLASH_ALIGN_PAD(struct ctap_resident_key))

/**
 * @brief State struct size with alignment padding
 */
#define CTAP_FLASH_STATE_SZ (sizeof(ctap_state_t) + \
                             CTAP_FLASH_ALIGN_PAD(ctap_state_t))

/**
 * @brief Write to flash memory and verify the written page against the given data
 *
 * @param[in] page      page to write to
 * @param[in] offset    offset into the page
 * @param[in] data      data to write and compare against
 * @param[in] len       length of data
 *
 * @return @ref ctap_status_codes_t
 */
int fido2_ctap_mem_write(const void *buf, uint32_t page, uint32_t offset, uint32_t size);

/**
 * @brief Read flashpage into buf
 *
 * Wrapper for @ref flashpage_read
 *
 * @param[in] page       page to read from
 * @param[in] buf        memory to write the page to
 *
 */
int fido2_ctap_mem_read(void *buf, uint32_t page, uint32_t offset, uint32_t size);

int fido2_ctap_mem_init(void);

uint16_t fido2_ctap_mem_get_max_rk_amount(void);

int fido2_ctap_mem_get_offset_of_rk_into_flashpage(uint16_t rk_idx);

int fido2_ctap_mem_get_flashpage_number_of_rk(uint16_t rk_idx);

#ifdef __cplusplus
}
#endif
#endif /* FIDO2_CTAP_CTAP_MEM_H */
/** @} */

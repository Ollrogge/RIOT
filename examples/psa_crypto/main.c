/*
 * Copyright (C) 2022 HAW Hamburg
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @brief       Example application for PSA Crypto
 *
 * @author      Lena Boeckmann <lena.boeckmann@haw-hamburg.de>
 *
 * @}
 */

#include <stdio.h>
#include "psa/crypto.h"

#include "xtimer.h"

extern void cipher_aes_128(void);
extern void psa_hmac_sha256(void);
extern void ecdsa(void);

#ifdef MULTIPLE_SE
extern void cipher_aes_128_sec_se(void);
extern void hmac_sha256_sec_se(void);
extern void ecdsa_sec_se(void);
#endif /* MULTIPLE_SE */

#define ENABLE_DEBUG    (1)
#include "debug.h"

int main(void)
{
    xtimer_sleep(0x2);
    psa_crypto_init();

    psa_hmac_sha256();
    cipher_aes_128();
    ecdsa();

#ifdef MULTIPLE_SE
    cipher_aes_128_sec_se();
    hmac_sha256_sec_se();
    ecdsa_sec_se();
#endif /* MULTIPLE_SE */

    DEBUG("DONE \n");
    puts("All Done");
    return 0;
}

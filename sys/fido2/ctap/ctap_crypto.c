/*
 * Copyright (C) 2021 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup fido2_ctap_crypto
 * @{
 * @file
 *
 * @author      Nils Ollrogge <nils.ollrogge@fu-berlin.de>
 * @}
 */

#include <string.h>

#include "assert.h"
#include "random.h"

#include "crypto/ciphers.h"
#include "crypto/modes/ccm.h"
#include "crypto/modes/cbc.h"
#include "crypto/aes.h"

#include "uECC.h"
#include "tiny-asn1.h"

#include "fido2/ctap/ctap_crypto.h"
#include "fido2/ctap.h"
#include "fido2/ctap/ctap_utils.h"

#if IS_ACTIVE(CONFIG_FIDO2_CTAP_SE_CREDS) || IS_ACTIVE(CONFIG_FIDO2_CTAP_SE_ENC_CREDS)
#include "atca_params.h"
#endif

#define ENABLE_DEBUG    (1)
#include "debug.h"

#define ECC_CURVE_BITS      (256)

/**
 * @brief Parse signature into ASN.1 DER format
 */
static int _sig_to_der_format(uint8_t *r, uint8_t *s, uint8_t *sig,
                              size_t *sig_len);

/**
 * @brief Random number generator
 *
 * wrapper for @ref fido2_ctap_crypto_prng
 */
static int _RNG(uint8_t *dest, unsigned size);

static void _configure_psa(void);

/**
 * @brief AES key id to encrypt credentials
 */
#if IS_ACTIVE(CONFIG_FIDO2_CTAP_SE_ENC_CREDS)
static psa_key_id_t _key_id;
#endif

int fido2_ctap_crypto_init(void)
{
#if IS_ACTIVE(CONFIG_FIDO2_CTAP_SE_CREDS) || IS_ACTIVE(CONFIG_FIDO2_CTAP_SE_ENC_CREDS)
    psa_status_t status = psa_crypto_init();

    if (status != PSA_SUCCESS) {
        DEBUG("psa_crypto_init failed: %ld\n", status);
        return CTAP1_ERR_OTHER;
    }

    DEBUG("ctap_crypto_init: PSA initialized \n");
#if IS_ACTIVE(CONFIG_FIDO2_CTAP_SE_ENC_CREDS)
    // initialize a new key every time because PSA only supports volatile key
    // storage at the moment
    int ret = fido2_ctap_crypto_gen_aeskey_se(&_key_id);

    if (ret != CTAP2_OK) {
        return ret;
    }
    DEBUG("ctap_crypto_init: AES key generated \n");
#endif
#endif

    uECC_set_rng(&_RNG);

    DEBUG("ctap_crypto_init: initialization done \n");

    return CTAP2_OK;
}

static int _RNG(uint8_t *dest, unsigned size)
{
    fido2_ctap_crypto_prng(dest, (size_t)size);
    return 1;
}

int fido2_ctap_crypto_prng(uint8_t *buf, size_t len)
{
    random_bytes(buf, len);
    return CTAP2_OK;
}

int fido2_ctap_crypto_sha256_init(sha256_context_t *ctx)
{
    sha256_init(ctx);
    return CTAP2_OK;
}

int fido2_ctap_crypto_sha256_update(sha256_context_t *ctx, const void *data, size_t len)
{
    sha256_update(ctx, data, len);
    return CTAP2_OK;
}

int fido2_ctap_crypto_sha256_final(sha256_context_t *ctx, void *digest)
{
    sha256_final(ctx, digest);
    return CTAP2_OK;
}

int fido2_ctap_crypto_sha256(const void *data, size_t len,
                             void *digest)
{
    sha256(data, len, digest);
    return CTAP2_OK;
}

int fido2_ctap_crypto_hmac_sha256_init(hmac_context_t *ctx, const void *key,
                                       size_t key_length)
{
    hmac_sha256_init(ctx, key, key_length);
    return CTAP2_OK;
}

int fido2_ctap_crypto_hmac_sha256_update(hmac_context_t *ctx, const void *data, size_t len)
{
    hmac_sha256_update(ctx, data, len);
    return CTAP2_OK;
}

int fido2_ctap_crypto_hmac_sha256_final(hmac_context_t *ctx, void *digest)
{
    hmac_sha256_final(ctx, digest);
    return CTAP2_OK;
}

int fido2_ctap_crypto_hmac_sha256(const void *key,
                                  size_t key_length, const void *data, size_t len,
                                  void *digest)
{
    hmac_sha256(key, key_length, data, len, digest);
    return CTAP2_OK;
}

int fido2_ctap_crypto_ecdh(uint8_t *out, size_t len,
                           uint8_t *pub_key, uint8_t *priv_key, size_t key_len)
{
    assert(len == CTAP_CRYPTO_KEY_SIZE);
    assert(key_len == CTAP_CRYPTO_KEY_SIZE);

    int ret;
    const struct uECC_Curve_t *curve = uECC_secp256r1();

    // temporary solution as long as psa does not support ECDH
#if IS_ACTIVE(CONFIG_FIDO2_CTAP_SE_CREDS)
    ret = uECC_shared_secret(pub_key + 1, priv_key, out, curve);
#else
    ret = uECC_shared_secret(pub_key, priv_key, out, curve);
#endif

    if (ret == 0) {
        return CTAP1_ERR_OTHER;
    }

    return CTAP2_OK;
}

int fido2_ctap_crypto_aes_enc(uint8_t *out, size_t *out_len, uint8_t *in,
                              size_t in_len, const uint8_t *key,
                              size_t key_len)
{
    assert(*out_len >= in_len);
    int ret;
    cipher_t cipher;
    uint8_t iv[16] = { 0 };

    ret = cipher_init(&cipher, CIPHER_AES, key, key_len);

    if (ret < 0) {
        return CTAP1_ERR_OTHER;
    }

    ret = cipher_encrypt_cbc(&cipher, iv, in, in_len, out);

    if (ret < 0) {
        return CTAP1_ERR_OTHER;
    }

    return CTAP2_OK;
}

int fido2_ctap_crypto_aes_dec(uint8_t *out, size_t *out_len, uint8_t *in,
                              size_t in_len, const uint8_t *key,
                              size_t key_len)
{
    assert(*out_len >= in_len);
    int ret;
    cipher_t cipher;
    uint8_t iv[16] = { 0 };

    ret = cipher_init(&cipher, CIPHER_AES, key, key_len);

    if (ret < 0) {
        return CTAP1_ERR_OTHER;
    }

    ret = cipher_decrypt_cbc(&cipher, iv, in, in_len, out);

    if (ret < 0) {
        return CTAP1_ERR_OTHER;
    }

    return CTAP2_OK;
}

#if IS_ACTIVE(CONFIG_FIDO2_CTAP_SE_ENC_CREDS)
int fido2_ctap_crypto_gen_aeskey_se(psa_key_id_t *key_id)
{
    uint8_t key[AES_KEY_SIZE_128];
    fido2_ctap_crypto_prng(key, sizeof(key));

    psa_status_t status = PSA_ERROR_DOES_NOT_EXIST;
    psa_key_attributes_t attr = psa_key_attributes_init();
    psa_key_lifetime_t lifetime = PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_LIFETIME_VOLATILE, PSA_ATCA_LOCATION_DEV0);
    psa_key_usage_t usage = PSA_KEY_USAGE_ENCRYPT | PSA_KEY_USAGE_DECRYPT;

    psa_set_key_lifetime(&attr, lifetime);
    psa_set_key_algorithm(&attr,PSA_ALG_CBC_NO_PADDING);
    psa_set_key_usage_flags(&attr, usage);
    psa_set_key_bits(&attr, PSA_BYTES_TO_BITS(AES_KEY_SIZE_128));
    psa_set_key_type(&attr, PSA_KEY_TYPE_AES);

    //status = psa_generate_key(&attr, key_id);

    status = psa_import_key(&attr, key, sizeof(key), key_id);

    if (status != PSA_SUCCESS) {
        DEBUG("AES 128 Key Generation failed: %ld\n", status);
        return CTAP1_ERR_OTHER;
    }

    return CTAP2_OK;
}

int fido2_ctap_crypto_aes_enc_se(uint8_t *out, size_t out_len, uint8_t *in,
                                size_t in_len)
{
    size_t _out_len = 0x0;
    psa_status_t status = psa_cipher_encrypt(_key_id, PSA_ALG_CBC_NO_PADDING, in,
                            in_len, out, out_len, &_out_len);

    if (status != PSA_SUCCESS) {
        DEBUG("AES 128 CBC Encrypt failed: %ld\nOutput Length: %d\n", status, _out_len);
        return CTAP1_ERR_OTHER;
    }

    return CTAP2_OK;
}

int fido2_ctap_crypto_aes_dec_se(uint8_t *out, size_t out_len, uint8_t *in,
                                size_t in_len)
{
    size_t _out_len = 0x0;
    psa_status_t status = psa_cipher_decrypt(_key_id, PSA_ALG_CBC_NO_PADDING, in, in_len,
                                out, out_len, &_out_len);

    if (status != PSA_SUCCESS) {
        DEBUG("AES 128 CBC Decrypt failed: %ld\nOutput Length: %d\n", status, _out_len);
        return CTAP1_ERR_OTHER;
    }

    return CTAP2_OK;
}
#endif /* CONFIG_FIDO2_CTAP_SE_ENC_CREDS */

int fido2_ctap_crypto_aes_ccm_enc(uint8_t *out, size_t out_len,
                                  const uint8_t *in, size_t in_len,
                                  uint8_t *auth_data, size_t auth_data_len,
                                  uint8_t mac_len, uint8_t length_encoding,
                                  const uint8_t *nonce, size_t nonce_len,
                                  const uint8_t *key, size_t key_len)
{
    assert(key_len == CTAP_CRED_KEY_LEN);

    cipher_t cipher;
    int ret;

    ret = cipher_init(&cipher, CIPHER_AES, key, key_len);

    if (ret != 1) {
        return CTAP1_ERR_OTHER;
    }

    ret = cipher_encrypt_ccm(&cipher, auth_data, auth_data_len, mac_len,
                             length_encoding, nonce, nonce_len,
                             in, in_len, out);

    if (ret < 0) {
        return CTAP1_ERR_OTHER;
    }

    return CTAP2_OK;
}

int fido2_ctap_crypto_aes_ccm_dec(uint8_t *out, size_t out_len,
                                  const uint8_t *in, size_t in_len,
                                  uint8_t *auth_data, size_t auth_data_len,
                                  uint8_t mac_len, uint8_t length_encoding,
                                  const uint8_t *nonce, size_t nonce_len,
                                  const uint8_t *key, size_t key_len)
{
    assert(key_len == CTAP_CRED_KEY_LEN);

    cipher_t cipher;
    int ret, len;

    ret = cipher_init(&cipher, CIPHER_AES, key, key_len);

    if (ret != 1) {
        return CTAP1_ERR_OTHER;
    }

    len = cipher_decrypt_ccm(&cipher, auth_data, auth_data_len,
                             mac_len, length_encoding, nonce, nonce_len,
                             in, in_len, out);

    if (len < 0) {
        return CTAP1_ERR_OTHER;
    }

    return CTAP2_OK;
}

int fido2_ctap_crypto_gen_keypair(uint8_t *pub_key, uint8_t *priv_key)
{
    int ret;
    const struct uECC_Curve_t *curve = uECC_secp256r1();

    // temporary solution as long as psa does not support ECDH
#if IS_ACTIVE(CONFIG_FIDO2_CTAP_SE_CREDS)
    ret = uECC_make_key(pub_key + 1, priv_key, curve);
#else
    ret = uECC_make_key(pub_key, priv_key, curve);
#endif
    if (ret == 0) {
        return CTAP1_ERR_OTHER;
    }

    return CTAP2_OK;
}

#if IS_ACTIVE(CONFIG_FIDO2_CTAP_SE_CREDS)
#define ECC_KEY_SIZE    (256)
int fido2_ctap_crypto_gen_keypair_se(ctap_crypto_pub_key_t *pub_key, psa_key_id_t *key_id, size_t len)
{
    psa_status_t status = PSA_ERROR_DOES_NOT_EXIST;
    psa_key_usage_t usage = PSA_KEY_USAGE_SIGN_HASH | PSA_KEY_USAGE_VERIFY_HASH;
    psa_key_type_t type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);
    psa_algorithm_t alg =  PSA_ALG_ECDSA(PSA_ALG_SHA_256);
    psa_key_bits_t bits = ECC_KEY_SIZE;
    psa_key_attributes_t key_attr = psa_key_attributes_init();
    psa_key_lifetime_t lifetime = PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(
        PSA_KEY_LIFETIME_VOLATILE, PSA_ATCA_LOCATION_DEV0);

    psa_set_key_lifetime(&key_attr, lifetime);
    psa_set_key_algorithm(&key_attr, alg);
    psa_set_key_usage_flags(&key_attr, usage);
    psa_set_key_type(&key_attr, type);
    psa_set_key_bits(&key_attr, bits);

    status = psa_generate_key(&key_attr, key_id);

    if (status != PSA_SUCCESS) {
        DEBUG("Primary SE Generate Key failed: %ld\n", status);
        return CTAP1_ERR_OTHER;
    }

    size_t pubkey_len;
    status = psa_export_public_key(*key_id, (uint8_t *)pub_key, len, &pubkey_len);

    if (status != PSA_SUCCESS) {
        printf("Primary SE Export Public Key failed: %ld\n", status);
        return CTAP1_ERR_OTHER;
    }

    DEBUG("fido2_ctap_crypto_gen_keypair_se success: public key length: %u, key_id: %lu \n",
        (unsigned)pubkey_len, *key_id);

    return CTAP2_OK;
}

int fido2_ctap_crypto_get_sig_se(uint8_t* hash, size_t hash_len, uint8_t *sig,
                                 size_t* sig_len, psa_key_id_t key_id)
{
    psa_status_t status = PSA_ERROR_DOES_NOT_EXIST;
    size_t _sig_len;
    /**
     * +1 to pad with leading zero to prevent integer from being interpreted as
     * negative (e.g. MSB of r >= 0x80)
     */
    uint8_t r[CTAP_CRYPTO_KEY_SIZE + 1] = { 0 };
    uint8_t s[CTAP_CRYPTO_KEY_SIZE + 1] = { 0 };
    int ret;

    psa_key_type_t type = PSA_KEY_TYPE_ECC_KEY_PAIR(PSA_ECC_FAMILY_SECP_R1);
    psa_algorithm_t alg =  PSA_ALG_ECDSA(PSA_ALG_SHA_256);
    psa_key_bits_t bits = ECC_CURVE_BITS;

    /**
     *  todo: find more elegant solution to avoid psa_error because sig_len
     *  isn't exactly 64 bytes but CTAP_CRYPTO_ES256_DER_MAX_SIZE (72) bytes
     */
    size_t sig_len_psa = PSA_SIGN_OUTPUT_SIZE(type, bits, alg);
    assert(*sig_len >= sig_len_psa);

    status = psa_sign_hash(key_id, PSA_ALG_ECDSA(PSA_ALG_SHA_256), hash,
                            hash_len, sig, sig_len_psa, &_sig_len);

    if (status != PSA_SUCCESS) {
        printf("Primary SE Sign hash failed: %ld\n", status);
        return CTAP1_ERR_OTHER;
    }

    DEBUG("SE Sign success, sig_len: %u \n", (unsigned)_sig_len);

    memcpy(r + 1, sig, CTAP_CRYPTO_KEY_SIZE);
    memcpy(s + 1, sig + CTAP_CRYPTO_KEY_SIZE, CTAP_CRYPTO_KEY_SIZE);

    ret = _sig_to_der_format(r, s, sig, sig_len);

    if (ret != CTAP2_OK) {
        return ret;
    }

    return CTAP2_OK;
}
#endif

int fido2_ctap_crypto_get_sig(uint8_t *hash, size_t hash_len, uint8_t *sig,
                              size_t *sig_len, const uint8_t *key,
                              size_t key_len)
{
    assert(*sig_len >= CTAP_CRYPTO_ES256_DER_MAX_SIZE);
    assert(key_len == CTAP_CRYPTO_KEY_SIZE);

    /**
     * +1 to pad with leading zero to prevent integer from being interpreted as
     * negative (e.g. MSB of r >= 0x80)
     */
    uint8_t r[CTAP_CRYPTO_KEY_SIZE + 1] = { 0 };
    uint8_t s[CTAP_CRYPTO_KEY_SIZE + 1] = { 0 };
    int ret;

    const struct uECC_Curve_t *curve = uECC_secp256r1();

    ret = uECC_sign(key, hash, hash_len, sig, curve);

    if (ret == 0) {
        return CTAP1_ERR_OTHER;
    }

    memcpy(r + 1, sig, CTAP_CRYPTO_KEY_SIZE);
    memcpy(s + 1, sig + CTAP_CRYPTO_KEY_SIZE, CTAP_CRYPTO_KEY_SIZE);

    ret = _sig_to_der_format(r, s, sig, sig_len);

    if (ret != CTAP2_OK) {
        return ret;
    }

    return CTAP2_OK;
}

static int _sig_to_der_format(uint8_t *r, uint8_t *s, uint8_t *sig,
                              size_t *sig_len)
{
    asn1_tree t;
    asn1_tree c1;
    asn1_tree c2;
    uint8_t pad_s, pad_r;
    int ret;

    /**
     * if MSB >= 0x80, pad with leading zero byte in order to have number
     * interpreted as positive.
     */
    pad_r = ((r[1] & 0x80) == 0x80);
    pad_s = ((s[1] & 0x80) == 0x80);

    memset(sig, 0, *sig_len);

    list_init(&t);
    list_init(&c1);
    list_init(&c2);

    t.type = ASN1_TYPE_SEQUENCE;

    c1.type = ASN1_TYPE_INTEGER;
    c1.length = 0x20 + pad_r;
    c1.data = pad_r ? r : r + 1;

    ret = add_child(&t, &c1);

    if (ret < 0) {
        return CTAP1_ERR_OTHER;
    }

    c2.type = ASN1_TYPE_INTEGER;
    c2.length = 0x20 + pad_s;
    c2.data = pad_s ? s : s + 1;

    ret = add_child(&t, &c2);

    if (ret < 0) {
        return CTAP1_ERR_OTHER;
    }

    *sig_len = der_encode(&t, sig, *sig_len);

    return CTAP2_OK;
}

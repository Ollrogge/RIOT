#include <string.h>
#include "fido2/ctap.h"
#include "fido2/ctap/ctap.h"
#include "fido2/ctap/transport/ctap_transport.h"
#include "fido2/ctap/ctap_mem.h"
#include "iolist.h"
#include "cond.h"
#include "event.h"
#include "fmt.h"
#include "gnrc_lorawan_internal.h"

#include "fido2/ctap/ctap_crypto.h"

#define ENABLE_DEBUG 1
#include "debug.h"

static void _set_state(fido_lora_state_t state);

iolist_t _data;
// byte 0 = type
uint8_t buf[0x1 + sizeof(ctap_resp_t)];

static void _join_accpt(event_t *arg);

// needs to be preconfigured to find credential
// const char rp_id[] = {"localhost"};
// uint8_t pub_js[] = {0x3b, 0x69, 0x58, 0x35, 0xef, 0x2a, 0x84, 0xc4, 0x73, 0x04, 0xcd, 0xf2, 0x66, 0xec, 0x99, 0xd2, 0xf9, 0x13, 0xcf, 0xfd, 0x7f, 0xd5, 0xb2, 0x1a, 0x32, 0x4e, 0xd5, 0x81, 0x63, 0x0f, 0x07, 0xf5, 0x78, 0xb0, 0x74, 0x1c, 0xd5, 0xf3, 0xbc, 0xab, 0x29, 0xa4, 0xf4, 0xf8, 0xf2, 0xd5, 0x62, 0x7f, 0x8b, 0xe0, 0xbe, 0x63, 0x1e, 0x47, 0xd9, 0xb2, 0x85, 0x11, 0x83, 0x79, 0xe1, 0x7f, 0x52, 0xbd};

// uint8_t priv_js[] = {0xf2, 0x93, 0x93, 0x97, 0x1f, 0x62, 0x2c, 0x8b, 0x1e, 0xb9, 0xec, 0x84, 0x6c, 0x8c, 0x6a, 0xe6, 0xa9, 0x5a, 0xe1, 0xc3, 0xbc, 0x76, 0x27, 0x65, 0xee, 0x7d, 0x1c, 0x18, 0xac, 0x85, 0x55, 0x61};

static event_t _join_accpt_event = {.handler = _join_accpt};

typedef struct
{
    ctap_req_t req;
    ctap_resp_t resp;
    fido_lora_state_t state;
    ctap_resident_key_t key;
} state_t;

static state_t _state = {
    .state = FIDO_LORA_GA_BEGIN};

static cond_t _cond = COND_INIT;
static mutex_t _lock = MUTEX_INIT;

void _set_state(fido_lora_state_t state)
{
    _state.state = state;
}

int gnrc_lorawan_fido_derive_root_keys(gnrc_lorawan_t *mac, uint8_t *deveui)
{
    uint32_t start = ztimer_now(ZTIMER_MSEC);

    uint8_t rp_id_hash[SHA256_DIGEST_LENGTH] = {0};
    uint8_t pub_js[0x40];
    sha256(CONFIG_FIDO2_LORAWAN_RP_ID, strlen(CONFIG_FIDO2_LORAWAN_RP_ID), rp_id_hash);
    ctap_resident_key_t *key = &_state.key;
    bool found = fido2_ctap_get_rk(key, rp_id_hash);

    if (!found)
    {
        DEBUG("Unable to find resident key for rp_id: %s \n", CONFIG_FIDO2_LORAWAN_RP_ID);
        return -1;
    }

    fmt_hex_bytes(pub_js, CONFIG_FIDO2_LORAWAN_PUB_JS);

    uint8_t secret[CTAP_CRYPTO_KEY_SIZE] = {0};

    int ret = fido2_ctap_crypto_ecdh(secret, sizeof(secret), pub_js, key->priv_key, sizeof(key->priv_key));

    if (ret != CTAP2_OK)
    {
        return -1;
    }

    uint8_t new_keys[SHA256_DIGEST_LENGTH] = {0};
    sha256(secret, sizeof(secret), new_keys);

    uint8_t *appkey = &new_keys[0];
    uint8_t *nwkkey = &new_keys[LORAMAC_APPKEY_LEN];

    memcpy(mac->ctx.appskey, appkey, LORAMAC_APPKEY_LEN);
    memcpy(mac->ctx.nwksenckey, nwkkey, LORAMAC_NWKKEY_LEN);

    gnrc_lorawan_generate_lifetime_session_keys(deveui, nwkkey,
                                                gnrc_lorawan_get_jsintkey(mac),
                                                gnrc_lorawan_get_jsenckey(mac));

    uint32_t end = ztimer_now(ZTIMER_MSEC);

    DEBUG("Root key derivation took: %lu \n", end - start);

    DEBUG("root key derivation done \n");
    DEBUG("Appkey: ");
    for (unsigned i = 0; i < LORAMAC_APPKEY_LEN; i++)
    {
        DEBUG("%02x", mac->ctx.appskey[i]);
    }
    DEBUG("\n");

    DEBUG("Nwkkey: ");
    for (unsigned i = 0; i < LORAMAC_APPKEY_LEN; i++)
    {
        DEBUG("%02x", mac->ctx.nwksenckey[i]);
    }
    DEBUG("\n");

    /*

    uint8_t pub_key[0x40] = {0};
    uint8_t priv_key[0x20];
    fido2_ctap_crypto_gen_keypair(pub_key, priv_key);
    DEBUG("Public key: {");
    for (int i = 0; i < 0x40; i++) {
        DEBUG("0x%02x, ", pub_key[i]);
    }
    DEBUG("}\n");
    DEBUG("Private key: {");
    for (int i = 0; i < 0x20; i++) {
        DEBUG("0x%02x, ", priv_key[i]);
    }
    DEBUG("}\n");
    */

    return 0;
}

fido_lora_state_t gnrc_lorawan_fido_get_state(void)
{
    return _state.state;
}

iolist_t *gnrc_lorawan_fido_join_req(void)
{
    ctap_resp_t *resp = &_state.resp;
    if (_state.state == FIDO_LORA_GA_BEGIN)
    {
        DEBUG("gnrc_lorawan_fido_join_req: GA_BEGIN \n");
        resp->status = FIDO_LORA_GA_BEGIN;

        _data.iol_next = NULL;
        _data.iol_base = resp;
        _data.iol_len = 0x1;

        if (IS_USED(CONFIG_FIDO2_LORAWAN_SAVE_PUB_KEY))
        {
            DEBUG("Sending public key in FIDO_LORA_GA_BEGIN \n");
            memcpy(resp->data, fido2_ctap_get_rk_pub_key(&_state.key), sizeof(ctap_crypto_pub_key_t));
            _data.iol_len += sizeof(ctap_crypto_pub_key_t);
        }
    }
    else
    {
        DEBUG("gnrc_lorawan_fido_join_req: GA_FINISH \n");

        if (resp->status == CTAP2_OK && resp->length > 0x0)
        {
            _data.iol_next = NULL;
            _data.iol_base = resp;
            // status code + data
            _data.iol_len = 0x1 + resp->length;
        }
        else
        {
            _data.iol_next = NULL;
            _data.iol_base = resp;
            // status code only
            _data.iol_len = 0x1;
        }

        resp->status = FIDO_LORA_GA_FINISH;

        /**
         * go back to initial state. Don't wait for server response due to
         * timeouts etc.
         */
        //_set_state(FIDO_LORA_GA_BEGIN);
    }

    return &_data;
}

int gnrc_lorawan_fido_join_accpt(uint8_t *data, size_t length)
{
    _state.req.buf = data;
    _state.req.len = length;
    _state.req.method = CTAP_GET_ASSERTION;

    uint32_t start = ztimer_now(ZTIMER_MSEC);

    event_queue_t *queue = fido2_ctap_transport_get_event_queue();

    /**
     * post to fido2 event queue to execute handler func inside fido2 thread due
     * to stack space
     */
    event_post(queue, &_join_accpt_event);

    mutex_lock(&_lock);
    cond_wait(&_cond, &_lock);
    mutex_unlock(&_lock);

    uint32_t end = ztimer_now(ZTIMER_MSEC);

    DEBUG("fido2 request processing took: %lu \n", end - start);
    // DEBUG("lora thread back \n");

    if (_state.resp.status != CTAP2_OK)
    {
        _set_state(FIDO_LORA_GA_BEGIN);
        return -1;
    }

    _set_state(FIDO_LORA_GA_FINISH);
    return 0;
}

static void _join_accpt(event_t *arg)
{
    (void)arg;

    size_t len = fido2_ctap_handle_request(&_state.req, &_state.resp);

    _state.resp.length = len;

    cond_signal(&_cond);

    DEBUG("fido2 resp: %u \n", _state.resp.status);
}

void gnrc_lorawan_reset_state(void)
{
    _set_state(FIDO_LORA_GA_BEGIN);
}

#include "findmy.h"

#include <string.h>

#include "ble_main.h"

#define NRF_LOG_MODULE_NAME findmy
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
NRF_LOG_MODULE_REGISTER();

// Apple offline-finding advertisement is a single Manufacturer Specific Data
// AD structure that fills a full 31-byte legacy advertising payload.
#define FINDMY_ADV_LEN 31

// TESTING AID: a Complete Local Name carried in the scan response so the beacon
// is easy to spot in a generic BLE scanner (e.g. nRF Connect). The primary
// advertisement is unchanged, so the FindMy network still works. Set to "" (or
// drop the scan response) for production -- real FindMy accessories are nameless
// and non-scannable for stealth.
#define FINDMY_TEST_SCAN_NAME "CU-FindMy"

static uint8_t m_pubkey[FINDMY_PUBKEY_LEN];
static bool    m_running = false;

void findmy_set_key(const uint8_t *pubkey) {
    memcpy(m_pubkey, pubkey, FINDMY_PUBKEY_LEN);
}

bool findmy_has_key(void) {
    for (uint8_t i = 0; i < FINDMY_PUBKEY_LEN; i++) {
        if (m_pubkey[i] != 0) {
            return true;
        }
    }
    return false;
}

// Derive the BLE address from the public key. The over-the-air address is the
// first 6 bytes of the key with the two most-significant bits forced to 1 (so it
// is a valid random-static address). ble_gap_addr_t.addr[] is little-endian, so
// the key's first byte maps to addr[5] (the transmitted MSB).
static void findmy_build_addr(uint8_t *addr6) {
    addr6[5] = m_pubkey[0] | 0xC0;
    addr6[4] = m_pubkey[1];
    addr6[3] = m_pubkey[2];
    addr6[2] = m_pubkey[3];
    addr6[1] = m_pubkey[4];
    addr6[0] = m_pubkey[5];
}

// Build the Apple offline-finding advertisement payload.
// Layout (see OpenHaystack / Macless-Haystack):
//   1e ff 4c 00 12 19 <status> <key[6..27]> <key[0]>>6> <hint>
static void findmy_build_advdata(uint8_t *buf) {
    uint8_t i = 0;
    buf[i++] = 0x1e;  // AD length: 30 bytes follow
    buf[i++] = 0xff;  // AD type: Manufacturer Specific Data
    buf[i++] = 0x4c;  // Company ID (Apple), little-endian
    buf[i++] = 0x00;
    buf[i++] = 0x12;  // Apple payload type: offline finding
    buf[i++] = 0x19;  // Apple payload length: 25
    buf[i++] = 0x00;  // status byte
    memcpy(&buf[i], &m_pubkey[6], 22);  // public key bytes 6..27
    i += 22;
    buf[i++] = m_pubkey[0] >> 6;  // top two bits of public key byte 0
    buf[i++] = 0x00;              // hint
}

// TESTING AID: build a scan response carrying a Complete Local Name AD so the
// beacon is identifiable in a BLE scanner. Returns the length (0 if name empty).
static uint8_t findmy_build_scanrsp(uint8_t *buf) {
    const char *name = FINDMY_TEST_SCAN_NAME;
    uint8_t name_len = (uint8_t) strlen(name);
    if (name_len == 0) {
        return 0;
    }
    uint8_t i = 0;
    buf[i++] = name_len + 1;  // AD length (type + name)
    buf[i++] = 0x09;          // AD type: Complete Local Name
    memcpy(&buf[i], name, name_len);
    i += name_len;
    return i;
}

uint32_t findmy_start(void) {
    uint8_t addr6[6];
    uint8_t advdata[FINDMY_ADV_LEN];
    uint8_t scanrsp[31];

    findmy_build_addr(addr6);
    findmy_build_advdata(advdata);
    uint8_t scanrsp_len = findmy_build_scanrsp(scanrsp);

    uint32_t err = ble_findmy_advertising_start(addr6, advdata, FINDMY_ADV_LEN, scanrsp, scanrsp_len);
    if (err == NRF_SUCCESS) {
        m_running = true;
        NRF_LOG_INFO("FindMy beacon started");
    } else {
        NRF_LOG_ERROR("FindMy beacon start failed: 0x%X", err);
    }
    return err;
}

uint32_t findmy_stop(void) {
    uint32_t err = ble_findmy_advertising_stop();
    if (err == NRF_SUCCESS) {
        m_running = false;
        NRF_LOG_INFO("FindMy beacon stopped");
    }
    return err;
}

bool findmy_is_running(void) {
    return m_running;
}

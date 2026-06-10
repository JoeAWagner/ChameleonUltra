#ifndef FINDMY_H
#define FINDMY_H

#include <stdint.h>
#include <stdbool.h>

// Length of the P-224 public key used by Apple's offline-finding network.
#define FINDMY_PUBKEY_LEN 28

/**@brief Store the 28-byte offline-finding public key to be advertised.
 *
 * @param[in] pubkey  Pointer to FINDMY_PUBKEY_LEN bytes of public key.
 */
void findmy_set_key(const uint8_t *pubkey);

/**@brief Returns true once a non-zero key has been provisioned. */
bool findmy_has_key(void);

/**@brief Start advertising the FindMy/offline-finding beacon.
 *
 * Stops the normal connectable advertising, switches the BLE address to the
 * key-derived random-static address, and begins non-connectable advertising.
 *
 * @return NRF_SUCCESS or a SoftDevice error code.
 */
uint32_t findmy_start(void);

/**@brief Stop the FindMy beacon and restore normal connectable advertising. */
uint32_t findmy_stop(void);

/**@brief Returns true while the FindMy beacon is active. */
bool findmy_is_running(void);

#endif

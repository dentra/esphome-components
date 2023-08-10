#pragma once

#include "miot.h"

namespace esphome {
namespace miot {

/**
 * Decrypt MiBeacon.
 *
 * @return true on success
 */
bool decrypt_mibeacon(MiBeacon &mib, const uint8_t *bindkey);

}  // namespace miot
}  // namespace esphome

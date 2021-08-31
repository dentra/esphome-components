#pragma once

#include "miot.h"

namespace esphome {
namespace miot {

/**
 * Decrypt MiBeacon version 4 and 5.
 *
 * @return true on success
 */
bool decrypt_mibeacon45(const MiotListener *listener, MiBeacon &mib);
/**
 * Decrypt MiBeacon version 2 and 3.
 *
 * @return true on success
 */
bool decrypt_mibeacon23(const MiotListener *listener, MiBeacon &mib);

}  // namespace miot
}  // namespace esphome

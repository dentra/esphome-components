#pragma once

#include "miot.h"

namespace esphome {
namespace miot {

/**
 * Dump MiBeacon via ESP_LOD.
 */
void dump(const char *const TAG, const MiBeacon &mib);

}  // namespace miot
}  // namespace esphome

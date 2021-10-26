#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "energy_tariff.h"
#include "energy_tariffs.h"

namespace esphome {
namespace energy_tariffs {

static const char *const TAG = "energy_tariff";
static const char *const GAP = "  ";

void EnergyTariff::dump_config() {
  LOG_SENSOR(GAP, "Energy Tariff", this);
  ESP_LOGCONFIG(TAG, "    Default: %s", this->is_default() ? "true" : "false");

  for (auto const &t : this->time_) {
    ESP_LOGCONFIG(TAG, "    Time: %02d:%02d-%02d:%02d", t.min / 60, t.min % 60, t.max / 60, t.max % 60);
  }
  ESP_LOGCONFIG(TAG, "    State: %f", this->state);
}

void EnergyTariff::setup() {
  const bool save_to_flash = this->save_to_flash_interval_ > 0;
  this->rtc_ = global_preferences->make_preference<float>(this->get_object_id_hash(), save_to_flash);

  float loaded;
  if (this->rtc_.load(&loaded)) {
    this->publish_state_and_save(loaded);
  } else {
    this->publish_state_and_save(0);
  }

#ifdef USE_API
  if (!this->service.empty()) {
    this->register_service(&EnergyTariff::publish_state_and_save, this->service, {"value"});
  }
#endif
}

void EnergyTariff::publish_state_and_save(float state) {
  ESP_LOGD(TAG, "'%s': Setting new state to %f", this->get_name().c_str(), state);
  this->publish_state(state);

  if (this->save_to_flash_interval_ > 0) {
    const uint32_t now = millis();
    if (now - this->last_save_ < this->save_to_flash_interval_) {
      return;
    }
    this->last_save_ = now;
  }
  this->rtc_.save(&state);
}

}  // namespace energy_tariffs
}  // namespace esphome

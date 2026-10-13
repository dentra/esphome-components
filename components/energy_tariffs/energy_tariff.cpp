#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "energy_tariff.h"
#include "energy_tariffs.h"

namespace esphome {
namespace energy_tariffs {

static const char *const TAG = "energy_tariff";
static const char *const GAP = "  ";
static const char *const DAYS_OF_WEEK[] = {"SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT"};

void EnergyTariff::dump_config() {
  LOG_SENSOR(GAP, "Energy Tariff", this);
  ESP_LOGCONFIG(TAG, "    Default: %s", this->is_default() ? "true" : "false");
  for (auto const &t : this->time_) {
    ESP_LOGCONFIG(TAG, "    Time: %02d:%02d-%02d:%02d", t.min / 60, t.min % 60, t.max / 60, t.max % 60);
  }
  if (this->days_of_week_ != 0) {
    std::string days;
    for (uint8_t i = 1; i <= 7; i++) {
      if (this->days_of_week_ & (1 << i)) {
        if (!days.empty()) {
          days += ",";
        }
        days += DAYS_OF_WEEK[i - 1];
      }
    }
    ESP_LOGCONFIG(TAG, "    Days of week: %s", days.c_str());
  }
  ESP_LOGCONFIG(TAG, "    State: %.2f", this->get_state());
}

void EnergyTariff::setup() {
  this->rtc_ = global_preferences->make_preference<float>(this->get_object_id_hash());

  float loaded;
  if (this->rtc_.load(&loaded)) {
    this->publish_state_and_save(loaded);
  } else {
    this->publish_state_and_save(this->initial_value_);
  }

#ifdef USE_API
  if (!this->service_.empty()) {
    this->register_service(&EnergyTariff::publish_state_and_save, this->service_, {"value"});
  }
#endif
}

void EnergyTariff::publish_state_and_save(float state) {
  ESP_LOGD(TAG, "'%s': Setting new state to %.2f", this->get_name().c_str(), state);
  this->publish_state(state);
  this->rtc_.save(&state);
}

}  // namespace energy_tariffs
}  // namespace esphome

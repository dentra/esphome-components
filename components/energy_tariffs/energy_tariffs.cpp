#include "esphome/core/log.h"
#include "esphome/components/time/real_time_clock.h"
#include "energy_tariffs.h"

namespace esphome {
namespace energy_tariffs {

static const char *const TAG = "energy_tariffs";

EnergyTariffs::~EnergyTariffs() {
  delete this->tariff_callback_;
  delete this->before_tariff_callback_;
}

void EnergyTariffs::dump_config() {
  ESP_LOGCONFIG(TAG, "Energy tariffs sensors");
  for (auto t : this->tariffs_) {
    t->dump_config();
  }
  LOG_NUMBER("", "Time offset", this->time_offset_);
  for (auto h : this->holidays_) {
    if (h >> 16) {
      ESP_LOGCONFIG(TAG, "  Holiday: %04u-%02u-%02u", unsigned(h >> 16), unsigned((h >> 8) & 0xFF), unsigned(h & 0xFF));
    } else {
      ESP_LOGCONFIG(TAG, "  Holiday: %02u.%02u", unsigned(h & 0xFF), unsigned((h >> 8) & 0xFF));
    }
  }
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR("  ", "Holiday", this->holiday_);
#endif
}

void EnergyTariffs::setup() {
  for (auto t : this->tariffs_) {
    t->setup();
  }

  if (this->time_offset_) {
    this->rtc_ = global_preferences->make_preference<float>(fnv1_hash(TAG));
    float loaded;
    if (this->rtc_.load(&loaded)) {
      auto call = this->time_offset_->make_call();
      call.set_value(loaded);
      call.perform();
    }
  }

  this->total_->add_on_state_callback([this](float state) { this->process_(state); });
}

void EnergyTariffs::loop() {
  auto time = this->time_->now();
  if (!time.is_valid()) {
    // time is not sync yet
    return;
  }

  if (this->time_offset_ && this->time_offset_->has_state()) {
    // sync time with mains
    time_t ts = time.timestamp + this->time_offset_->state;
    time = ESPTime::from_epoch_local(ts);
  }

  auto ct = this->get_tariff_(time);
  if (ct != this->current_tariff_) {
    this->process_(this->total_->get_state());
    this->current_tariff_ = ct;
    if (this->tariff_callback_) {
      this->tariff_callback_->call(ct);
    }
  }

  if (this->before_tariff_callback_) {
    time.increment_second();
    ct = this->get_tariff_(time);
    if (ct == this->current_tariff_) {
      this->before_tariff_triggered_ = false;
    } else if (!this->before_tariff_triggered_) {
      this->before_tariff_triggered_ = true;
      this->before_tariff_callback_->call();
    }
  }
}

void EnergyTariffs::process_(float total) {
  if (!this->current_tariff_) {
    return;
  }
  if (std::isnan(total)) {
    return;
  }
  if (std::isnan(this->last_total_)) {
    this->last_total_ = total;
    return;
  }
  float x = total - this->last_total_;
  if (x > 0.005f) {
    this->current_tariff_->add(x);
    this->last_total_ = total;
  }
}

EnergyTariff *EnergyTariffs::get_tariff_(const ESPTime &time) const {
  const uint8_t day_of_week = this->is_holiday_(time) ? this->holiday_as_ : time.day_of_week;
  EnergyTariff *def{};
  for (auto t : this->tariffs_) {
    if (t->is_default()) {
      def = t;
    } else if (t->is_active(time, day_of_week)) {
      return t;
    }
  }
  return def;
}

bool EnergyTariffs::is_holiday_(const ESPTime &time) const {
#ifdef USE_BINARY_SENSOR
  if (this->holiday_ && this->holiday_->has_state() && this->holiday_->state) {
    return true;
  }
#endif
  if (this->holidays_.empty()) {
    return false;
  }
  const auto annual = encode_date_(time.day_of_month, time.month, 0);
  const auto exact = encode_date_(time.day_of_month, time.month, time.year);
  for (auto h : this->holidays_) {
    if (h == annual || h == exact) {
      return true;
    }
  }
  return false;
}

}  // namespace energy_tariffs
}  // namespace esphome

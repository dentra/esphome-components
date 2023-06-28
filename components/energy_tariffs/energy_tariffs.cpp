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
  LOG_NUMBER("", "Time offset", this->time_offset_)
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
    if (ct != this->current_tariff_) {
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
  EnergyTariff *def{};
  for (auto t : this->tariffs_) {
    if (t->is_default()) {
      def = t;
    } else if (t->time_in_range(time)) {
      return t;
    }
  }
  return def;
}

void EnergyTariffs::add_on_tariff_callback(std::function<void(Sensor *)> &&callback) {
  if (!this->tariff_callback_) {
    this->tariff_callback_ = new CallbackManager<void(Sensor *)>();
  }
  this->tariff_callback_->add(std::move(callback));
}

void EnergyTariffs::add_on_before_tariff_callback(std::function<void()> &&callback) {
  if (!this->before_tariff_callback_) {
    this->before_tariff_callback_ = new CallbackManager<void()>();
  }
  this->before_tariff_callback_->add(std::move(callback));
}

}  // namespace energy_tariffs
}  // namespace esphome

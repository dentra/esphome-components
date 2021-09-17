#include "esphome/core/log.h"
#include "esphome/components/time/real_time_clock.h"
#include "energy_tariffs.h"

namespace esphome {
namespace energy_tariffs {

static const char *const TAG = "energy_tariffs";
static const char *const GAP = "  ";

EnergyTariffs::~EnergyTariffs() {
  delete this->tariff_callback_;
  delete this->before_tariff_callback_;
}

void EnergyTariffs::dump_config() {
  ESP_LOGCONFIG(TAG, "Energy tariffs sensors");
  for (auto t : this->tariffs_) {
    t->dump_config();
  }
  if (this->time_offset_ != 0) {
    ESP_LOGCONFIG(GAP, "Time offset: %d", this->time_offset_);
  }
#ifdef USE_API
  if (!this->time_offset_service_.empty()) {
    ESP_LOGCONFIG(GAP, "Time offset service: %s", this->time_offset_service_.c_str());
  }
#endif
}

void EnergyTariffs::setup() {
  for (auto t : this->tariffs_) {
    t->setup();
  }

#ifdef USE_API
  float loaded;
  if (this->rtc_.load(&loaded)) {
    this->set_time_offset(loaded);
  } else {
    this->rtc_.save(&this->time_offset_);
  }

  if (!this->time_offset_service_.empty()) {
    this->register_service(&EnergyTariffs::set_time_offset_, this->time_offset_service_, {"value"});
  }
#endif

  this->total_->add_on_state_callback([this](float state) { this->process_(state); });
}

void EnergyTariffs::loop() {
  auto t = this->time_->now();
  if (!t.is_valid()) {
    // time is not sync yet
    return;
  }

  if (this->time_offset_ != 0) {
    // sync time with mains
    time_t ts = t.timestamp + this->time_offset_;
    t = time::ESPTime::from_epoch_local(ts);
  }

  auto ct = this->get_tariff_(t);
  if (ct != this->current_tariff_) {
    auto total = this->total_->get_state();
    if (!isnan(total)) {
      this->process_(total);
    }
    this->current_tariff_ = ct;
    if (this->tariff_callback_) {
      this->tariff_callback_->call(ct);
    }
  }

  if (this->before_tariff_callback_) {
    t.increment_second();
    ct = this->get_tariff_(t);
    if (ct != this->current_tariff_) {
      this->before_tariff_callback_->call();
    }
  }
}

void EnergyTariffs::process_(float total) {
  if (!this->current_tariff_) {
    return;
  }
  float x = total - this->last_total_;
  this->current_tariff_->add(x);
  this->last_total_ = total;
}

EnergyTariff *EnergyTariffs::get_tariff_(const time::ESPTime &time) const {
  EnergyTariff *def{nullptr};
  for (auto t : this->tariffs_) {
    if (t->is_default()) {
      def = t;
    } else if (t->time_in_range(time)) {
      return t;
    }
  }
  return def;
}

#ifdef USE_API
void EnergyTariffs::set_time_offset_(float value) {
  int seconds = value;
  ESP_LOGD(TAG, "Setting time offset to %d seconds", seconds);
  this->set_time_offset(seconds);
  this->rtc_.save(&value);
}
#endif

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

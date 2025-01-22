#include "esphome/core/log.h"
#include "esphome/core/hal.h"
#include "energy_statistics.h"

namespace esphome {
namespace energy_statistics {

static const char *const TAG = "energy_statistics";

static const char *const PREF_V1 = "energy_statistics";
static const char *const PREF_V2 = "energy_statistics_v2";

void EnergyStatistics::dump_config() {
  ESP_LOGCONFIG(TAG, "Energy statistics sensors");
  if (this->energy_today_) {
    LOG_SENSOR("  ", "Energy Today", this->energy_today_);
  }
  if (this->energy_yesterday_) {
    LOG_SENSOR("  ", "Energy Yesterday", this->energy_yesterday_);
  }
  if (this->energy_week_) {
    LOG_SENSOR("  ", "Energy Week", this->energy_week_);
  }
  if (this->energy_month_) {
    LOG_SENSOR("  ", "Energy Month", this->energy_month_);
  }
  if (this->energy_year_) {
    LOG_SENSOR("  ", "Energy Year", this->energy_year_);
  }
}

void EnergyStatistics::setup() {
  this->total_->add_on_state_callback([this](float state) { this->process_(state); });

  this->pref_ = global_preferences->make_preference<energy_data_t>(fnv1_hash(PREF_V2));
  bool loaded = this->pref_.load(&this->energy_);
  if (!loaded) {
    // migrating from v1 data
    loaded = global_preferences->make_preference<energy_data_v1_t>(fnv1_hash(PREF_V1)).load(&this->energy_);
    if (loaded) {
      this->energy_.start_year = this->energy_.start_month;
      // save as v2
      this->pref_.save(&this->energy_);
      global_preferences->sync();
    }
  }
  if (loaded) {
    auto total = this->total_->get_state();
    if (!std::isnan(total)) {
      this->process_(total);
    }
  }
}

void EnergyStatistics::loop() {
  const auto t = this->time_->now();
  if (!t.is_valid()) {
    // time is not sync yet
    return;
  }

  const auto total = this->total_->get_state();
  if (std::isnan(total)) {
    // total is not published yet
    return;
  }

  // update stats first time or on next day
  if (t.day_of_year == this->energy_.current_day_of_year) {
    // nothing to do
    return;
  }

  this->energy_.start_yesterday = this->energy_.start_today;

  this->energy_.start_today = total;

  if (this->energy_.current_day_of_year != 0) {
    // at specified day of week we start a new week calculation
    if (t.day_of_week == this->energy_week_start_day_) {
      this->energy_.start_week = total;
    }
    // at first day of month we start a new month calculation
    if (t.day_of_month == 1) {
      this->energy_.start_month = total;
    }
  }

  // Intitialize all sensors. https://github.com/dentra/esphome-components/issues/65
  if (this->energy_week_ && std::isnan(this->energy_.start_week)) {
    this->energy_.start_week = this->energy_.start_yesterday;
  }
  if (this->energy_month_ && std::isnan(this->energy_.start_month)) {
    this->energy_.start_month = this->energy_.start_yesterday;
  }
  if (this->energy_year_ && std::isnan(this->energy_.start_year)) {
    this->energy_.start_year = this->energy_.start_yesterday;
  }

  this->energy_.current_day_of_year = t.day_of_year;

  this->process_(total);
}

void EnergyStatistics::process_(float total) {
  if (this->energy_today_ && !std::isnan(this->energy_.start_today)) {
    this->energy_today_->publish_state(total - this->energy_.start_today);
  }

  if (this->energy_yesterday_ && !std::isnan(this->energy_.start_yesterday)) {
    this->energy_yesterday_->publish_state(this->energy_.start_today - this->energy_.start_yesterday);
  }

  if (this->energy_week_ && !std::isnan(this->energy_.start_week)) {
    this->energy_week_->publish_state(total - this->energy_.start_week);
  }

  if (this->energy_month_ && !std::isnan(this->energy_.start_month)) {
    this->energy_month_->publish_state(total - this->energy_.start_month);
  }
  
  if (this->energy_year_ && !std::isnan(this->energy_.start_year)) {
    this->energy_year_->publish_state(total - this->energy_.start_year);
  }
  
  this->pref_.save(&this->energy_);
}

}  // namespace energy_statistics
}  // namespace esphome

#include "esphome/core/log.h"
#include "energy_statistics.h"

#ifndef isnan
using std::isnan;
#endif

namespace esphome {
namespace energy_statistics {

static const char *const TAG = "energy_statistics";
static const char *const GAP = "  ";

void EnergyStatistics::dump_config() {
  ESP_LOGCONFIG(TAG, "Energy statistics sensors");
  if (this->energy_today_) {
    LOG_SENSOR(GAP, "Energy Today", this->energy_today_);
  }
  if (this->energy_yesterday_) {
    LOG_SENSOR(GAP, "Energy Yesterday", this->energy_yesterday_);
  }
  if (this->energy_week_) {
    LOG_SENSOR(GAP, "Energy Week", this->energy_week_);
  }
  if (this->energy_month_) {
    LOG_SENSOR(GAP, "Energy Month", this->energy_month_);
  }
}

void EnergyStatistics::setup() {
  this->total_->add_on_state_callback([this](float state) { this->process_(state); });

  /*
    this->pref_ = global_preferences->make_preference<energy_data_t>(this->get_object_id_hash(), false);

    energy_data_t loaded{}
    if (this->pref_.load(&loaded)) {
      this->energy_ = loaded;
      auto total = this->total_->get_state();
      if (!isnan(total)) {
        this->process_(total);
      }
    }
  */
}

void EnergyStatistics::loop() {
  auto t = this->time_->now();
  if (!t.is_valid()) {
    // time is not sync yet
    return;
  }

  auto total = this->total_->get_state();
  if (isnan(total)) {
    // total is not published yet
    return;
  }

  if (t.day_of_year == this->energy_.current_day_of_year_) {
    // nothing to do
    return;
  }

  this->energy_.start_yesterday = this->energy_.start_today;

  this->energy_.start_today = total;

  if (this->energy_.current_day_of_year_ != 0) {
    // at specified day of week we start a new week calculation
    if (t.day_of_week == this->energy_week_start_day_) {
      this->energy_.start_week = total;
    }
  }

  this->energy_.current_day_of_year_ = t.day_of_year;

  this->process_(total);
}

void EnergyStatistics::process_(float total) {
  if (this->energy_today_ && !isnan(this->energy_.start_today)) {
    this->energy_today_->publish_state(total - this->energy_.start_today);
  }

  if (this->energy_yesterday_ && !isnan(this->energy_.start_yesterday)) {
    this->energy_yesterday_->publish_state(this->energy_.start_today - this->energy_.start_yesterday);
  }

  if (this->energy_week_ && !isnan(this->energy_.start_week)) {
    this->energy_week_->publish_state(total - this->energy_.start_week);
  }

  if (this->energy_month_ && !isnan(this->energy_.start_month)) {
    this->energy_month_->publish_state(total - this->energy_.start_month);
  }

  // this->pref_.save(&(this->energy_));
}

}  // namespace energy_statistics
}  // namespace esphome

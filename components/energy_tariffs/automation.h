#pragma once

#include "esphome/core/component.h"
#include "esphome/core/automation.h"
#include "esphome/components/sensor/sensor.h"
#include "energy_tariffs.h"

namespace esphome {
namespace energy_tariffs {

using sensor::Sensor;

// TODO use Parented<EnergyStatistics>

class TariffChangeTrigger : public Trigger<Sensor *> {
public:
  explicit TariffChangeTrigger(EnergyTariffs *parent) {
    parent->add_on_tariff_callback([this](Sensor *value) { this->trigger(value); });
  }
};

class BeforeTariffChangeTrigger : public Trigger<> {
public:
  explicit BeforeTariffChangeTrigger(EnergyTariffs *parent) {
    parent->add_on_before_tariff_callback([this] { this->trigger(); });
  }
};

/*
template<typename... Ts> class TariffIsCondition : public Condition<Ts...>, public Parented<EnergyTariffs>{
public:
  TariffIsCondition(EnergyStatistics *parent) : Parented(parent) {}

  TEMPLATABLE_VALUE(EnergyTariff *, tariff)

  bool check(Ts... x) override { return this->parent_->get_current_tariff() == this->tariff_.value(x...); }

};
*/

/*
template<typename... Ts> class TariffSetAction : public Action<Ts...> {
public:
  explicit TariffSetAction(EnergyTariff *tariff) : tariff_(tariff) {}

  TEMPLATABLE_VALUE(float, state)

  void play(Ts... x) override {
    auto v = this->state_.value(x...);
    this->tariff_->publish_state(v);
  }

protected:
  EnergyTariff *tariff_;
};
*/

} // namespace energy_statistics
} // namespace esphome

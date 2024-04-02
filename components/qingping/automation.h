#pragma once

#include "esphome/core/automation.h"

#include "qingping.h"

namespace esphome {
namespace qingping {

class QingpingStateTrigger : public Trigger<const QPDataPoint &> {
 public:
  explicit QingpingStateTrigger(Qingping *qp) {
    qp->add_on_state_callback([this](const QPDataPoint &state) {
      this->trigger(state);
      // FIXME do something to pass false result if state was not processed
      return true;
    });
  }
};

}  // namespace qingping
}  // namespace esphome

#pragma once
#ifdef USE_VPORT_UART

#include "esphome/core/component.h"

#include "vport.h"

namespace esphome {
namespace vport {

#define VPORT_UART_LOG(port_name) VPORT_LOG(port_name)

// interface UartIO {
//   using reader_type = etl::delegate<void(const frame_spec_t &frame, size_t size)>;
//   void set_reader(reader_type &&reader);
//   void write(const uint8_t *data, size_t size);
//   void poll();
// };
template<class io_t, class frame_spec_t, class component_t = Component>
class VPortUARTComponent : public VPortIO<io_t, frame_spec_t>, public component_t {
  static_assert(std::is_base_of<Component, component_t>::value, "component_t must derived from Component class");

 public:
  explicit VPortUARTComponent(io_t *io) : VPortIO<io_t, frame_spec_t>(io) {}

  void setup() override {
    this->defer([this] { this->fire_ready(); });
  }

  void loop() override { this->io_->poll(); }
};

}  // namespace vport
}  // namespace esphome
#endif

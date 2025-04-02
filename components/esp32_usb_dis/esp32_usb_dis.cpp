#ifdef USE_ESP32

#include "soc/soc.h"
#include "esp_idf_version.h"

#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
#if defined(USE_ESP32_VARIANT_ESP32S3)
#include "soc/usb_device_reg.h"
#elif defined(USE_ESP32_VARIANT_ESP32C3)
#include "soc/usb_serial_jtag_reg.h"
#else
#error "Unsuppored ESP32 platform"
#endif
#endif

#include "esp32_usb_dis.h"

namespace esphome {
namespace esp32_usb_dis {

void pre_setup() {
#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 4, 0)
#if defined(USE_ESP32_VARIANT_ESP32S3)
  CLEAR_PERI_REG_MASK(USB_DEVICE_CONF0_REG, USB_DEVICE_USB_PAD_ENABLE);
#elif defined(USE_ESP32_VARIANT_ESP32C3)
  CLEAR_PERI_REG_MASK(USB_SERIAL_JTAG_CONF0_REG, USB_SERIAL_JTAG_DP_PULLUP);
#endif
#endif
}

}  // namespace esp32_usb_dis
}  // namespace esphome
#endif

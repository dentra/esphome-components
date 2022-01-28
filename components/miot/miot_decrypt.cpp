#include <cstring>
#include <inttypes.h>
#include "esphome/core/log.h"
#include "mbedtls/ccm.h"
#include "miot_decrypt.h"

namespace esphome {
namespace miot {

static const char *const TAG = "miot_decrypt";

bool decrypt_mibeacon45(const MiotListener *listener, MiBeacon &mib) {
  const uint8_t *data = mib.object.data.data();
  size_t size = mib.object.data.size();
  if (size > sizeof(RawBLEObject)) {
    ESP_LOGW(TAG, "%12" PRIX64 " [%04X] Encrypted data is too large", listener->get_address(),
             listener->get_product_id());
    return false;
  }

  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);

  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, listener->get_bindkey(),
                               sizeof(bindkey_t) * 8);  // 8 bits per byte
  if (ret != 0) {
    ESP_LOGW(TAG, "%12" PRIX64 " [%04X] mbedtls_ccm_setkey failed: %d", listener->get_address(),
             listener->get_product_id(), ret);
    mbedtls_ccm_free(&ctx);
    return false;
  }

  struct {
    esp_bd_addr_t mac;
    uint16_t product_id;
    uint32_t random_number;
  } PACKED nonce;
  memcpy(nonce.mac, mac_reverse(listener->get_address()), sizeof(esp_bd_addr_t));
  nonce.product_id = mib.product_id;
  nonce.random_number = mib.random_number;

  const uint8_t *iv = reinterpret_cast<const uint8_t *>(&nonce);
  const size_t iv_len = sizeof(nonce);
  const uint8_t add[] = {0x11};
  const size_t add_len = sizeof(add);
  const uint8_t *tag = reinterpret_cast<const uint8_t *>(&mib.message_integrity_check);
  const size_t tag_len = sizeof(mib.message_integrity_check);

  uint8_t result[sizeof(RawBLEObject)];
  ret = mbedtls_ccm_auth_decrypt(&ctx, size, iv, iv_len, add, add_len, data, result, tag, tag_len);
  mbedtls_ccm_free(&ctx);
  if (ret != 0) {
    ESP_LOGW(TAG, "%12" PRIX64 " [%04X] mbedtls_ccm_auth_decrypt failed: %d", listener->get_address(),
             listener->get_product_id(), ret);
    ESP_LOGW(TAG, "   mac: " ESP_BD_ADDR_STR, nonce.mac[5], nonce.mac[4], nonce.mac[3], nonce.mac[2], nonce.mac[1],
             nonce.mac[0]);
    ESP_LOGW(TAG, "   key: %s", format_hex_pretty(listener->get_bindkey(), sizeof(bindkey_t)).c_str());
    ESP_LOGW(TAG, "    iv: %s", format_hex_pretty(iv, iv_len).c_str());
    ESP_LOGW(TAG, "   add: %s", format_hex_pretty(add, add_len).c_str());
    ESP_LOGW(TAG, "   tag: %s", format_hex_pretty(tag, tag_len).c_str());
    ESP_LOGW(TAG, "  data: %s", format_hex_pretty(data, size).c_str());
    return false;
  }

  mib.object = BLEObject(reinterpret_cast<RawBLEObject *>(result));

  return true;
}

bool decrypt_mibeacon23(const MiotListener *listener, MiBeacon &mib) {
  const uint8_t *data = mib.object.data.data();
  size_t size = mib.object.data.size();
  if (size > sizeof(RawBLEObject)) {
    ESP_LOGW(TAG, "%12" PRIX64 " [%04X] Encrypted data is too large", listener->get_address(),
             listener->get_product_id());
    return false;
  }

  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);

  // key format is: bindkey[:6] "8d3d3c97" bindkey[6:]
  bindkey_t key = {0, 0, 0, 0, 0, 0, 0x8d, 0x3d, 0x3c, 0x97, 0, 0, 0, 0, 0, 0};
  memcpy(key + 0x0, listener->get_bindkey() + 0, 6);
  memcpy(key + 0xA, listener->get_bindkey() + 6, 6);

  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, sizeof(bindkey_t) * 8);  // 8 bits per byte
  if (ret != 0) {
    ESP_LOGW(TAG, "%12" PRIX64 " [%04X] mbedtls_ccm_setkey failed: %d", listener->get_address(),
             listener->get_product_id(), ret);
    mbedtls_ccm_free(&ctx);
    return false;
  }

  struct {
    FrameControl frame_control;
    uint16_t product_id;
    uint32_t random_number;
    uint8_t mac[sizeof(esp_bd_addr_t) - 1];  // only 5 bytes of mac
  } PACKED nonce;
  memcpy(nonce.mac, &listener->get_address(), sizeof(nonce.mac));
  nonce.frame_control = mib.frame_control;
  nonce.product_id = mib.product_id;
  nonce.random_number = mib.random_number;

  const uint8_t *iv = reinterpret_cast<const uint8_t *>(&nonce);
  const size_t iv_len = sizeof(nonce);
  const uint8_t add[] = {0x11};
  const size_t add_len = sizeof(add);

  uint8_t result[sizeof(RawBLEObject)] = {};
  ret = mbedtls_ccm_star_auth_decrypt(&ctx, size, iv, iv_len, add, add_len, data, result, nullptr, 0);
  mbedtls_ccm_free(&ctx);
  if (ret != 0) {
    ESP_LOGW(TAG, "%12" PRIX64 " [%04X] mbedtls_ccm_star_auth_decrypt failed: %d", listener->get_address(),
             listener->get_product_id(), ret);
    ESP_LOGW(TAG, "   key: %s", format_hex_pretty(key, sizeof(bindkey_t)).c_str());  // legacy bindkey size is 12 bytes
    ESP_LOGW(TAG, "    iv: %s", format_hex_pretty(iv, iv_len).c_str());
    ESP_LOGW(TAG, "   add: %s", format_hex_pretty(add, add_len).c_str());
    ESP_LOGW(TAG, "  data: %s", format_hex_pretty(data, size).c_str());
    return false;
  }

  auto obj = reinterpret_cast<RawBLEObject *>(result);
  if (obj->data_len > size - sizeof(obj->data_len)) {
    ESP_LOGW(TAG, "%12" PRIX64 " [%04X] decrypt_mibeacon23 failed", listener->get_address(),
             listener->get_product_id());
    return false;
  }

  mib.object = BLEObject(obj);

  return true;
}

}  // namespace miot
}  // namespace esphome

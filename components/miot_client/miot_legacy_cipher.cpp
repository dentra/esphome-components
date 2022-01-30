#include "esphome/core/log.h"
#include "esphome/core/helpers.h"
#include "miot_legacy_cipher.h"

namespace esphome {
namespace miot_client {

inline const uint8_t *mac_reverse(const uint64_t &mac) { return reinterpret_cast<const uint8_t *>(&mac); }

namespace cipher {

std::array<uint8_t, 8> mix_a(const uint8_t *mac, const uint16_t product_id) {
  return {
      mac[0], mac[2], mac[5], static_cast<uint8_t>(product_id & 0xFF), static_cast<uint8_t>(product_id & 0xFF),
      mac[4], mac[5], mac[1],
  };
}

std::array<uint8_t, 8> mix_a(const uint64_t &mac, const uint16_t product_id) {
  return mix_a(mac_reverse(mac), product_id);
}

std::array<uint8_t, 8> mix_b(const uint8_t *mac, const uint16_t product_id) {
  return {
      mac[0], mac[2], mac[5], static_cast<uint8_t>((product_id >> 8) & 0xFF),
      mac[4], mac[0], mac[5], static_cast<uint8_t>((product_id >> 0) & 0xFF),
  };
}

std::array<uint8_t, 8> mix_b(const uint64_t &mac, const uint16_t product_id) {
  return mix_b(mac_reverse(mac), product_id);
}

std::array<uint8_t, 256> cipher_init(const uint8_t *key, const std::size_t key_size) {
  std::array<uint8_t, 256> perm;
  for (std::size_t i = 0; i < perm.size(); i++) {
    perm[i] = i & 0xFF;
  }
  int j = 0;
  for (std::size_t i = 0; i < perm.size(); i++) {
    j += perm[i] + key[i % key_size];
    j = j & 0xFF;
    auto v = perm[i];
    perm[i] = perm[j];
    perm[j] = v;
  }
  return perm;
}

// More information: https://github.com/drndos/mikettle
std::vector<uint8_t> cipher(const uint8_t *key, const std::size_t key_size, const uint8_t *data,
                            const std::size_t data_size) {
  auto perm = cipher_init(key, key_size);
  int index1 = 0;
  int index2 = 0;
  std::vector<uint8_t> output;
  for (std::size_t i = 0; i < data_size; i++) {
    index1 = index1 + 1;
    index1 = index1 & 0xFF;
    index2 += perm[index1];
    index2 = index2 & 0xFF;
    auto v = perm[index1];
    perm[index1] = perm[index2];
    perm[index2] = v;
    auto idx = perm[index1] + perm[index2];
    idx = idx & 0xFF;
    auto outputByte = data[i] ^ perm[idx];
    output.push_back(outputByte & 0xFF);
  }
  return output;
}

std::array<uint8_t, 12> generate_random_token() {
  std::array<uint8_t, 12> token;
  for (int i = 0; i < token.size(); i++) {
    token[i] = random_uint32();
  }
  return token;
}

}  // namespace cipher
}  // namespace miot_client
}  // namespace esphome

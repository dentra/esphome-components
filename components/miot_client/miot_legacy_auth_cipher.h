#pragma once

#include <vector>

namespace esphome {
namespace miot_client {
namespace cipher {

std::vector<uint8_t> mix_a(const uint8_t *mac, const uint16_t product_id);
std::vector<uint8_t> mix_a(const uint64_t &mac, const uint16_t product_id);

std::vector<uint8_t> mix_b(const uint8_t *mac, const uint16_t product_id);
std::vector<uint8_t> mix_b(const uint64_t &mac, const uint16_t product_id);

std::vector<uint8_t> cipher(const std::vector<uint8_t> &key, const uint8_t *input, int size);
template<typename T> std::vector<uint8_t> cipher(const std::vector<uint8_t> &key, const T &input) {
  return cipher(key, input.data(), input.size());
}

std::vector<uint8_t> generate_random_token();

}  // namespace cipher
}  // namespace miot_client
}  // namespace esphome

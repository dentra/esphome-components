#pragma once

#include <vector>

namespace esphome {
namespace miot_ylxx0xyl_pair {
namespace cipher {

std::vector<uint8_t> mix_a(uint8_t *mac, uint16_t product_id);
std::vector<uint8_t> mix_b(uint8_t *mac, uint16_t product_id);
std::vector<uint8_t> cipher(const std::vector<uint8_t> &key, const uint8_t *input, int size);
std::vector<uint8_t> cipher(const std::vector<uint8_t> &key, const std::vector<uint8_t> &input);
std::vector<uint8_t> generate_random_token();

}  // namespace cipher
}  // namespace miot_pair_ylxx0xyl
}  // namespace esphome

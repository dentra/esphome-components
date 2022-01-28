#pragma once

#include <vector>
#include <array>

namespace esphome {
namespace miot_client {
namespace cipher {

std::array<uint8_t, 8> mix_a(const uint8_t *mac, const uint16_t product_id);
std::array<uint8_t, 8> mix_a(const uint64_t &mac, const uint16_t product_id);

std::array<uint8_t, 8> mix_b(const uint8_t *mac, const uint16_t product_id);
std::array<uint8_t, 8> mix_b(const uint64_t &mac, const uint16_t product_id);

std::vector<uint8_t> cipher(const uint8_t *key, const std::size_t key_size, const uint8_t *data,
                            const std::size_t size);
inline std::vector<uint8_t> cipher(const std::vector<uint8_t> &key, const uint8_t *data, const std::size_t size) {
  return cipher(key.data(), key.size(), data, size);
}
template<size_t K>
inline std::vector<uint8_t> cipher(const std::array<uint8_t, K> &key, const uint8_t *data, const std::size_t size) {
  return cipher(key.data(), key.size(), data, size);
}
inline std::vector<uint8_t> cipher(const std::vector<uint8_t> &key, const std::vector<uint8_t> &data) {
  return cipher(key.data(), key.size(), data.data(), data.size());
}
template<size_t N>
inline std::vector<uint8_t> cipher(const std::vector<uint8_t> &key, const std::array<uint8_t, N> &data) {
  return cipher(key.data(), key.size(), data.data(), data.size());
}
template<size_t K, size_t N>
inline std::vector<uint8_t> cipher(const std::array<uint8_t, K> &key, const std::array<uint8_t, N> &data) {
  return cipher(key.data(), key.size(), data.data(), data.size());
}
template<size_t K>
inline std::vector<uint8_t> cipher(const std::array<uint8_t, K> &key, const std::vector<uint8_t> &data) {
  return cipher(key.data(), key.size(), data.data(), data.size());
}
inline std::vector<uint8_t> cipher(const std::vector<uint8_t> &key, const uint32_t data) {
  return cipher(key.data(), key.size(), reinterpret_cast<const uint8_t *>(std::addressof(data)), sizeof(data));
}
template<size_t K> inline std::vector<uint8_t> cipher(const std::array<uint8_t, K> &key, const uint32_t data) {
  return cipher(key.data(), key.size(), reinterpret_cast<const uint8_t *>(std::addressof(data)), sizeof(data));
}

std::array<uint8_t, 12> generate_random_token();

}  // namespace cipher
}  // namespace miot_client
}  // namespace esphome

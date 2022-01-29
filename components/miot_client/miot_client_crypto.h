#pragma once
#include "esphome/core/helpers.h"

namespace esphome {
namespace miot_client {

struct SessionNonce {
  uint32_t iv;
  uint32_t reserved;  // always 0
  uint16_t seq;
  uint16_t seq_hi;  // always 0
  SessionNonce(uint32_t iv, uint16_t seq) : iv(iv), reserved(0), seq(seq), seq_hi(0){};
} PACKED;

/**
 * @brief This function encrypts a buffer using CCM.
 *
 * @param iv The initialization vector (nonce). This must be a readable buffer of at least iv_len Bytes.
 * @param iv_len The length of the nonce in Bytes: 7, 8, 9, 10, 11, 12, or 13.
 *               The length L of the message length field is 15 - iv_len.
 * @param add The additional data field.
 *            If add_len is greater than zero, add must be a readable buffer of at least that length.
 * @param add_len The length of additional data in Bytes. This must be less than `2^16 - 2^8`.
 * @param input The buffer holding the input data.
 *              If length is greater than zero, input must be a readable buffer of at least that length.
 * @param input_size The length of the input data in Bytes.
 * @param output The buffer holding the output data.
 *               If length is greater than zero, output must be a writable buffer of at least that length.
 * @param tag The buffer holding the authentication field. This must be a readable buffer of at least tag_len Bytes.
 * @param tag_len The length of the authentication field to generate in Bytes: 4, 6, 8, 10, 12, 14 or 16.
 *
 * @return 0 on success.
 */
int session_encrypt(const uint8_t *key, size_t key_size, const uint8_t *iv, size_t iv_size, const uint8_t *add,
                    size_t add_size, const uint8_t *input, size_t input_size, uint8_t *output, uint8_t *tag,
                    size_t tag_size);

inline int session_encrypt(const uint8_t *key, size_t key_size, const uint8_t *iv, size_t iv_size, const uint8_t *input,
                           size_t input_size, uint8_t *output, uint8_t *tag, size_t tag_size) {
  return session_encrypt(key, key_size, iv, iv_size, nullptr, 0, input, input_size, output, tag, tag_size);
}

inline int session_encrypt(const uint8_t *key, size_t key_size, const SessionNonce &nonce, const uint8_t *input,
                           size_t input_size, uint8_t *output, uint8_t *tag, size_t tag_size) {
  return session_encrypt(key, key_size, reinterpret_cast<const uint8_t *>(&nonce), sizeof(nonce), nullptr, 0, input,
                         input_size, output, tag, tag_size);
}

int session_decrypt(const uint8_t *key, size_t key_size, const uint8_t *iv, size_t iv_size, const uint8_t *add,
                    size_t add_size, const uint8_t *input, size_t input_size, uint8_t *output, const uint8_t *tag,
                    size_t tag_size);

inline int session_decrypt(const uint8_t *key, size_t key_size, const uint8_t *iv, size_t iv_size, const uint8_t *input,
                           size_t input_size, uint8_t *output, const uint8_t *tag, size_t tag_size) {
  return session_decrypt(key, key_size, iv, iv_size, nullptr, 0, input, input_size, output, tag, tag_size);
}

inline int session_decrypt(const uint8_t *key, size_t key_size, const SessionNonce &nonce, const uint8_t *input,
                           size_t input_size, uint8_t *output, const uint8_t *tag, size_t tag_size) {
  return session_decrypt(key, key_size, reinterpret_cast<const uint8_t *>(&nonce), sizeof(nonce), nullptr, 0, input,
                         input_size, output, tag, tag_size);
}

}  // namespace miot_client
}  // namespace esphome

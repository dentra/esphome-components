#include <cstring>
#include "mbedtls/ccm.h"
#include "mbedtls/md.h"
#include "miot_client_crypto.h"

#ifndef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef CEIL_DIV
#define CEIL_DIV(a, b) (((a) + (b) -1) / (b))
#endif
/*
extern "C" unsigned int mbedtls_hkdf(const mbedtls_md_info_t *md_info, const unsigned char *salt, unsigned int salt_len,
                                     const unsigned char *key, unsigned int key_len, const unsigned char *info,
                                     unsigned int info_len, unsigned char *out, unsigned int out_len) {
  unsigned char PRK[32];
  unsigned char T_n[32];
  unsigned int loop;
  unsigned int temp_len;

  // Step 1: HKDF-Extract(salt, IKM) -> PRK
  if (salt == NULL) {
    const unsigned char null_salt[32] = {0};
    mbedtls_md_hmac(md_info, null_salt, 32, key, key_len, PRK);
  } else {
    mbedtls_md_hmac(md_info, salt, salt_len, key, key_len, PRK);
  }

  // Step 2: HKDF-Expand(PRK, info, L) -> OKM
  // T(0) = empty string (zero length)
  // T(1) = HMAC-Hash(PRK, T(0) | info | 0x01)
  // T(2) = HMAC-Hash(PRK, T(1) | info | 0x02)
  // T(3) = HMAC-Hash(PRK, T(2) | info | 0x03)

  unsigned char temp[32 + info_len + 1];
  std::memset(temp, 0, 32 + info_len + 1);
  loop = CEIL_DIV(out_len, 32);

  for (int i = 0; i < loop; i++) {
    if (i == 0) {
      temp_len = 0;
    } else {
      std::memcpy(temp, T_n, 32);
      temp_len = 32;
    }

    std::memcpy(temp + temp_len, info, info_len);
    temp_len += info_len;

    temp[temp_len] = i + 1;
    temp_len += 1;

    mbedtls_md_hmac(md_info, PRK, 32, temp, temp_len, T_n);

    std::memcpy(out + 32 * i, T_n, MIN(32, out_len));
    out_len -= 32;
  }

  return 0;
}
*/
namespace esphome {
namespace miot_client {

int session_encrypt(const uint8_t *key, size_t key_size, const uint8_t *iv, size_t iv_size, const uint8_t *add,
                    size_t add_size, const uint8_t *input, size_t input_size, uint8_t *output, uint8_t *tag,
                    size_t tag_size) {
  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);
  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, key_size * 8);  // 8 bits per byte
  if (ret != 0) {
    return ret;
  }
  ret = mbedtls_ccm_encrypt_and_tag(&ctx, input_size, iv, iv_size, add, add_size, input, output, tag, tag_size);
  mbedtls_ccm_free(&ctx);
  return ret;
}

int session_decrypt(const uint8_t *key, size_t key_size, const uint8_t *iv, size_t iv_size, const uint8_t *add,
                    size_t add_size, const uint8_t *input, size_t input_size, uint8_t *output, const uint8_t *tag,
                    size_t tag_size) {
  mbedtls_ccm_context ctx;
  mbedtls_ccm_init(&ctx);
  int ret = mbedtls_ccm_setkey(&ctx, MBEDTLS_CIPHER_ID_AES, key, key_size * 8);  // 8 bits per byte
  if (ret != 0) {
    return ret;
  }
  ret = mbedtls_ccm_auth_decrypt(&ctx, input_size, iv, iv_size, add, add_size, input, output, tag, tag_size);
  mbedtls_ccm_free(&ctx);
  return ret;
}

}  // namespace miot_client
}  // namespace esphome

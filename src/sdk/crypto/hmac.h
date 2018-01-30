#ifndef DSA_SDK_CRYPTO_HMAC_H_
#define DSA_SDK_CRYPTO_HMAC_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <vector>

#include <openssl/hmac.h>

#include "util/buffer.h"

namespace dsa {
class HMAC {
 private:
  HMAC_CTX *ctx;
  bool initialized;

 public:
  HMAC(const std::vector<uint8_t> &to_hash) throw(const std::runtime_error &);
  ~HMAC();

  void update(const std::vector<uint8_t> &data) throw(
      const std::runtime_error &);
  std::vector<uint8_t> digest() throw(const std::runtime_error &);
};
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_HMAC_H_
#ifndef DSA_SDK_CRYPTO_HASH_H_
#define DSA_SDK_CRYPTO_HASH_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <openssl/evp.h>
#include <vector>
#include "util/buffer.h"

namespace dsa {
class Hash {
 private:
  EVP_MD_CTX *mdctx;
  bool finalized;

 public:
  explicit Hash() throw(const std::runtime_error &);
  ~Hash();

  void update(const std::vector<uint8_t> &data);
  string_ digest_base64() throw(const std::runtime_error &);
};
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_HASH_H_
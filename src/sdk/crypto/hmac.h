#ifndef DSA_SDK_CRYPTO_HMAC_H_
#define DSA_SDK_CRYPTO_HMAC_H_

#include <vector>

#include <openssl/hmac.h>

#include "util/buffer.h"

namespace dsa {
class HMAC {
 private:
  HMAC_CTX *ctx;
  bool initialized;

 public:
  HMAC(const char *alg, Buffer& to_hash) throw(const std::runtime_error &);
  ~HMAC();

  void init(const char *alg, Buffer& to_hash) throw(const std::runtime_error &);
  void update(const Buffer& data) throw(const std::runtime_error &);
  BufferPtr digest() throw(const std::runtime_error &);
};
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_HMAC_H_
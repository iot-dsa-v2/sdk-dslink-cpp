#ifndef DSA_SDK_CRYPTO_HMAC_H_
#define DSA_SDK_CRYPTO_HMAC_H_

#include <vector>

#include <openssl/hmac.h>

#include "dsa/util.h"

namespace dsa {
class HMAC {
 private:
  HMAC_CTX *ctx;
  bool initialized;

 public:
  HMAC(const char *alg, Buffer& to_hash);
  ~HMAC();

  void init(const char *alg, Buffer& to_hash);
  void update(const Buffer& data);
  BufferPtr digest();
};
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_HMAC_H_
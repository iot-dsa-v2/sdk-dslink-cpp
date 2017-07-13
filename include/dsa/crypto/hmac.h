#ifndef DSA_SDK_CRYPTO_HMAC_H_
#define DSA_SDK_CRYPTO_HMAC_H_

#include <vector>

#include <openssl/hmac.h>

#include "dsa/util.h"

namespace dsa {
class hmac {
 private:
  // HMAC_CTX *ctx;
  HMAC_CTX ctx;
  bool initialized;

 public:
  hmac(const char *alg, ByteBuffer& to_hash);
  ~hmac();

  void init(const char *alg, ByteBuffer& to_hash);
  void update(ByteBuffer& data);
  std::shared_ptr<ByteBuffer> digest();
};
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_HMAC_H_
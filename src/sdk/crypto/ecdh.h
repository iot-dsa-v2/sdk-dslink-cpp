#ifndef DSA_SDK_CRYPTO_ECDH_H_
#define DSA_SDK_CRYPTO_ECDH_H_

#include <vector>
#include <memory>

#include <openssl/ec.h>

#include "dsa/util.h"

namespace dsa {
class ECDH {
 private:
  EC_KEY *key;
  const EC_GROUP *group;
  bool is_key_valid_for_curve(BIGNUM *private_key);

 public:
  ECDH(const char *curve);
  ~ECDH();

  BufferPtr get_private_key() const;
  BufferPtr get_public_key() const;
  BufferPtr compute_secret(Buffer& public_key) const;
  void set_private_key_hex(const char *data);
};
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_ECDH_H_
#ifndef DSA_SDK_CRYPTO_ECDH_H_
#define DSA_SDK_CRYPTO_ECDH_H_

#include <vector>

#include <openssl/ec.h>

#include "util/buffer.h"

namespace dsa {
class ECDH {
 private:
  EC_KEY *key;
  const EC_GROUP *group;
  bool is_key_valid_for_curve(BIGNUM *private_key) throw(const std::runtime_error &);

 public:
  ECDH(const char *curve) throw(const std::runtime_error &);
  ~ECDH();

  BufferPtr get_private_key() const throw(const std::runtime_error &);
  BufferPtr get_public_key() const throw(const std::runtime_error &);
  BufferPtr compute_secret(Buffer& public_key) const throw(const std::runtime_error &);
  void set_private_key_hex(const char *data) throw(const std::runtime_error &);
};
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_ECDH_H_
#ifndef DSA_SDK_CRYPTO_ECDH_H_
#define DSA_SDK_CRYPTO_ECDH_H_

#include <vector>

#include <openssl/ec.h>

#include "util/buffer.h"

namespace dsa {
class ECDH {
 public:
  static const char *curve_name;

 private:
  EC_KEY *key;
  const EC_GROUP *group;
  bool is_key_valid_for_curve(BIGNUM *private_key) throw(
      const std::runtime_error &);

  void generate_key() throw(const std::runtime_error &);

 public:
  ECDH() throw(const std::runtime_error &);
  ECDH(const ECDH &ecdh);
  ECDH & operator=(const ECDH &ecdh);
  ~ECDH();

  std::string get_private_key() const throw(const std::runtime_error &);
  std::string get_public_key() const throw(const std::runtime_error &);
  std::string compute_secret(const std::string &public_key) const
      throw(const std::runtime_error &);
  void set_private_key_hex(const char *data) throw(const std::runtime_error &);
};
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_ECDH_H_
#ifndef DSA_SDK_CRYPTO_ECDH_H_
#define DSA_SDK_CRYPTO_ECDH_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <stdexcept>
#include <vector>

#include <openssl/ec.h>

namespace dsa {
class ECDH {
 public:
  static const char *curve_name;

  static ECDH *from_file(const char *path);

 private:
  EC_KEY *key;
  const EC_GROUP *group;
  bool is_key_valid_for_curve(BIGNUM *private_key) throw(
      const std::runtime_error &);

  void generate_key() throw(const std::runtime_error &);

  std::vector<uint8_t> _public_key_cache;
  void cache_public_key();

  void init_private_key(BIGNUM *priv) throw(const std::runtime_error &);

  string_ _dsid_cached_prefix;
  string_ _dsid_cache;

 public:
  ECDH() throw(const std::runtime_error &);
  ECDH(uint8_t *data, size_t size);
  ECDH(const ECDH &ecdh);
  ECDH &operator=(const ECDH &ecdh);
  ~ECDH();

  const string_ & get_dsid(const string_ &prefix);

  const std::vector<uint8_t> get_private_key() const
      throw(const std::runtime_error &);
  const std::vector<uint8_t> &get_public_key() const {
    return _public_key_cache;
  };
  std::vector<uint8_t> compute_secret(const std::vector<uint8_t> &public_key)
      const throw(const std::runtime_error &);

  void set_private_key_hex(const char *data) throw(const std::runtime_error &);
};
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_ECDH_H_
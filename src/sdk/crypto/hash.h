#ifndef DSA_SDK_CRYPTO_HASH_H_
#define DSA_SDK_CRYPTO_HASH_H_

#include <vector>

#include <openssl/evp.h>

#include "dsa/util.h"

namespace dsa {
class Hash {
 private:
  EVP_MD_CTX *mdctx;
  bool finalized;

  // hack-ish static initialization
  class Init {
   public:
    Init() { OpenSSL_add_all_digests(); }
  };
  Init init;

 public:
  explicit Hash(const char *hash_type) throw(const std::runtime_error &);
  ~Hash();

  void update(const Buffer& data);
  std::string digest_base64() throw(const std::runtime_error &);
};
}  // namespace dsa

#endif  // DSA_SDK_CRYPTO_HASH_H_
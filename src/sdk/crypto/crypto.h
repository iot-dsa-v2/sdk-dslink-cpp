#ifndef DSA_SDK_CRYPTO_H_
#define DSA_SDK_CRYPTO_H_

#include <array>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <openssl/ec.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>

namespace dsa {
class ecdh {
 private:
  EC_KEY *key;
  const EC_GROUP *group;
  bool is_key_valid_for_curve(BIGNUM *private_key);

 public:
  ecdh(const char *curve);
  ~ecdh();

  std::vector<uint8_t> get_private_key();
  std::vector<uint8_t> get_public_key();
  std::vector<uint8_t> compute_secret(std::vector<uint8_t> public_key);
  void set_private_key_hex(const char *data);
};

class hash {
 private:
  EVP_MD_CTX mdctx;
  bool finalized;

  // hack-ish static initialization
  class Init {
   public:
    Init() { OpenSSL_add_all_digests(); }
  };
  Init init;

 public:
  hash(const char *hash_type);
  ~hash();

  void update(std::vector<uint8_t> data);
  std::string digest_base64();
};

class hmac {
 private:
  // HMAC_CTX *ctx;
  HMAC_CTX ctx;
  bool initialized;

 public:
  hmac(const char *alg, std::vector<uint8_t> to_hash);
  ~hmac();

  void init(const char *alg, std::vector<uint8_t> to_hash);
  void update(std::vector<uint8_t> data);
  std::vector<uint8_t> digest();
};

std::string base64url(std::string str);
std::string base64_decode(std::string const &encoded_string);
std::string base64_encode(unsigned char const *uint8_ts_to_encode,
                          unsigned int in_len);
std::vector<uint8_t> gen_salt(int len);
std::vector<uint8_t> hex2bin(const char *src);
}  // namespace dsa

template <typename T>
inline std::ostream &operator<<(std::ostream &os, const std::vector<T> &v) {
  os << "[";
  if (v.size() > 0) {
    for (int i = 0; i < v.size() - 1; ++i) os << v[i] << ", ";
    os << v[v.size() - 1];
  }
  os << "]";
  return os;
}

inline std::ostream &operator<<(std::ostream &os, const std::vector<uint8_t> &v) {
  std::stringstream ss;
  ss << "[";
  if (v.size() > 0) {
    for (int i = 0; i < v.size() - 1; ++i) {
      ss << "x";
      if (v[i] < 0x10) ss << 0;
      ss << std::hex << (uint)v[i] << std::dec << ", ";
    }
    uint last = v[v.size() - 1];
    ss << "x" << (last < 0x10 ? "0" : "") << std::hex << last << std::dec;
  }
  ss << "]";
  return os << ss.str();
}

template <typename T, int S>
inline std::ostream &operator<<(std::ostream &os, const std::array<T, S> &v) {
  os << "[";
  if (v.size() > 0) {
    for (int i = 0; i < v.size() - 1; ++i) os << v[i] << ", ";
    os << v[v.size() - 1];
  }
  os << "]";
  return os;
}

template <int S>
inline std::ostream &operator<<(std::ostream &os,
                                const std::array<uint8_t, S> &v) {
  std::stringstream ss;
  ss << "[";
  if (v.size() > 0) {
    for (int i = 0; i < v.size() - 1; ++i) {
      ss << "x";
      if (v[i] < 0x10) ss << 0;
      ss << std::hex << (uint)v[i] << std::dec << ", ";
    }
    uint last = v[v.size() - 1];
    ss << "x" << (last < 0x10 ? "0" : "") << std::hex << last << std::dec;
  }
  ss << "]";
  return os << ss.str();
}

#endif  // DSA_SDK_CRYPTO_H_
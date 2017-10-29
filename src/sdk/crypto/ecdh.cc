#include "dsa_common.h"

#include "ecdh.h"

#include <openssl/ecdh.h>
#include <openssl/objects.h>

template <typename T, typename U>
inline void CHECK_NE(T a, U b) throw(const std::runtime_error &) {
  if (a == b) throw std::runtime_error("Something went wrong, can't be equal.");
}

namespace dsa {

const char *ECDH::curve_name = "prime256v1";

ECDH::ECDH() throw(const std::runtime_error &) {
  generate_key();
  cache_public_key();
}

ECDH::ECDH(const ECDH &ecdh) {
  int nid = OBJ_sn2nid(curve_name);
  key = EC_KEY_new_by_curve_name(nid);
  EC_KEY_copy(key, ecdh.key);
  group = EC_KEY_get0_group(key);
  cache_public_key();
}

ECDH::ECDH(uint8_t *data, size_t size) {
  BIGNUM *priv = BN_bin2bn(data, size, nullptr);
  init_private_key(priv);
}

ECDH::~ECDH() { EC_KEY_free(key); }

ECDH &ECDH::operator=(const ECDH &ecdh) {
  EC_KEY_free(key);

  int nid = OBJ_sn2nid(curve_name);
  key = EC_KEY_new_by_curve_name(nid);
  EC_KEY_copy(key, ecdh.key);
  group = EC_KEY_get0_group(key);
  return *this;
}
void ECDH::generate_key() throw(const std::runtime_error &) {
  int nid = OBJ_sn2nid(curve_name);
  if (nid == NID_undef) throw std::runtime_error("invalid curve name");
  key = EC_KEY_new_by_curve_name(nid);
  if (EC_KEY_generate_key(key) == 0)
    throw std::runtime_error("failed to generate ecdh key");
  group = EC_KEY_get0_group(key);
}

const std::vector<uint8_t> ECDH::get_private_key() const
    throw(const std::runtime_error &) {
  const BIGNUM *priv = EC_KEY_get0_private_key(key);
  if (priv == nullptr) throw std::runtime_error("private key not set");
  size_t size = BN_num_bytes(priv);

  std::vector<uint8_t> out = {0};
  out.resize(32);

  size_t start_pos = 32 - size;

  if (size != BN_bn2bin(priv, &out[start_pos])) {
    throw std::runtime_error("private key couldn't be retrieved");
  }

  return std::move(out);
}

void ECDH::init_private_key(BIGNUM *priv) throw(const std::runtime_error &) {
  if (!is_key_valid_for_curve(priv)) {
    BN_free(priv);
    throw std::runtime_error("invalid key for curve");
  }

  int result = EC_KEY_set_private_key(key, priv);
  BN_free(priv);

  if (result == 0)
    throw std::runtime_error("failed to convert BN to private key");

  // To avoid inconsistency, clear the current public key in-case computing
  // the new one fails for some reason.
  EC_KEY_set_public_key(key, nullptr);

  const BIGNUM *priv_key = EC_KEY_get0_private_key(key);
  CHECK_NE(priv_key, nullptr);

  EC_POINT *pub = EC_POINT_new(group);
  CHECK_NE(pub, nullptr);

  if (EC_POINT_mul(group, pub, priv_key, nullptr, nullptr, nullptr) == 0) {
    EC_POINT_free(pub);
    throw std::runtime_error("Failed to generate ecdh public key");
  }

  if (EC_KEY_set_public_key(key, pub) == 0) {
    EC_POINT_free(pub);
    return throw std::runtime_error("Failed to set generated public key");
  }
  EC_POINT_free(pub);

  cache_public_key();
}
void ECDH::cache_public_key() {
  const EC_POINT *pub = EC_KEY_get0_public_key(key);
  if (pub == nullptr) throw std::runtime_error("Couldn't get public key");

  size_t size;
  point_conversion_form_t form = EC_GROUP_get_point_conversion_form(group);

  size = EC_POINT_point2oct(group, pub, form, nullptr, 0, nullptr);
  if (size == 0) throw std::runtime_error("Couldn't get public key");

  _public_key_cache.resize(size);

  size_t r = EC_POINT_point2oct(group, pub, form, &_public_key_cache[0], size,
                                nullptr);
  if (r != size) {
    throw std::runtime_error("Couldn't get public key");
  }
}

bool ECDH::is_key_valid_for_curve(BIGNUM *private_key) throw(
    const std::runtime_error &) {
  if (group == nullptr) throw std::runtime_error("group cannot be null");
  if (private_key == nullptr)
    throw std::runtime_error("private key cannot be null");
  if (BN_cmp(private_key, BN_value_one()) < 0) return false;

  BIGNUM *order = BN_new();
  if (order == nullptr)
    throw std::runtime_error("something went wrong, order can't be null");
  bool result = (EC_GROUP_get_order(group, order, nullptr) != 0) &&
                (BN_cmp(private_key, order) < 0);
  BN_free(order);
  return result;
}

void ECDH::set_private_key_hex(const char *data) throw(
    const std::runtime_error &) {
  BIGNUM *priv = BN_new();
  BN_hex2bn(&priv, data);
  init_private_key(priv);
}

std::vector<uint8_t> ECDH::compute_secret(
    const std::vector<uint8_t> &public_key) const
    throw(const std::runtime_error &) {
  EC_POINT *pub = EC_POINT_new(group);
  int r = EC_POINT_oct2point(group, pub, public_key.data(), public_key.size(),
                             nullptr);
  if ((r == 0) || pub == nullptr)
    throw std::runtime_error("secret couldn't be computed with given key");

  // NOTE: field_size is in bits
  int field_size = EC_GROUP_get_degree(group);
  size_t size = ((size_t)field_size + 7) / 8;
  std::vector<uint8_t> out;
  out.resize(size);

  r = ECDH_compute_key(&out[0], size, pub, key, nullptr);
  EC_POINT_free(pub);
  if (r == 0) {
    throw std::runtime_error("secret couldn't be computed with given key");
  }

  return std::move(out);
}

}  // namespace dsa

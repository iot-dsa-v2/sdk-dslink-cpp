#include "dsa_common.h"

#include "ecdh.h"

#include <openssl/ecdh.h>
#include <openssl/objects.h>

template<typename T, typename U>
inline void CHECK_NE(T a, U b) throw(const std::runtime_error &) {
  if (a == b) throw std::runtime_error("Something went wrong, can't be equal.");
}

namespace dsa {

ECDH::ECDH(const char *curve) throw(const std::runtime_error &) {
  int nid = OBJ_sn2nid(curve);
  if (nid == NID_undef) throw std::runtime_error("invalid curve name");
  key = EC_KEY_new_by_curve_name(nid);
  if (EC_KEY_generate_key(key) == 0)
    throw std::runtime_error("failed to generate ecdh key");
  group = EC_KEY_get0_group(key);
}

ECDH::~ECDH() { EC_KEY_free(key); }

BufferPtr ECDH::get_private_key() const throw(const std::runtime_error &) {
  const BIGNUM *priv = EC_KEY_get0_private_key(key);
  if (priv == nullptr) throw std::runtime_error("private key not set");
  int size = BN_num_bytes(priv);

  auto *out = new uint8_t[size];

  if (size != BN_bn2bin(priv, out)) {
    delete[] out;
    throw std::runtime_error("private key couldn't be retrieved");
  }

  return std::move(make_intrusive_<Buffer>(out, size, size));
}

BufferPtr ECDH::get_public_key() const throw(const std::runtime_error &) {
  const EC_POINT *pub = EC_KEY_get0_public_key(key);
  if (pub == nullptr) throw std::runtime_error("Couldn't get public key");

  size_t size;
  point_conversion_form_t form = EC_GROUP_get_point_conversion_form(group);

  size = EC_POINT_point2oct(group, pub, form, nullptr, 0, nullptr);
  if (size == 0) throw std::runtime_error("Couldn't get public key");

  auto *out = new uint8_t[size];

  size_t r = EC_POINT_point2oct(group, pub, form, out, size, nullptr);
  if (r != size) {
    delete[] out;
    throw std::runtime_error("Couldn't get public key");
  }

  return std::move(make_intrusive_<Buffer>(out, size, size));
}

bool ECDH::is_key_valid_for_curve(BIGNUM *private_key) throw(const std::runtime_error &) {
  if (group == nullptr) throw std::runtime_error("group cannot be null");
  if (private_key == nullptr)
    throw std::runtime_error("private key cannot be null");
  if (BN_cmp(private_key, BN_value_one()) < 0) return false;

  BIGNUM *order = BN_new();
  if (order == nullptr)
    throw std::runtime_error("something went wrong, order can't be null");
  bool result = EC_GROUP_get_order(group, order, nullptr) &&
      (BN_cmp(private_key, order) < 0);
  BN_free(order);
  return result;
}

void ECDH::set_private_key_hex(const char *data) throw(const std::runtime_error &) {
  BIGNUM *priv = BN_new();
  BN_hex2bn(&priv, data);
  if (!is_key_valid_for_curve(priv))
    throw std::runtime_error("invalid key for curve");

  int result = EC_KEY_set_private_key(key, priv);
  BN_free(priv);

  if (result == 0) throw std::runtime_error("failed to convert BN to private key");

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
}

BufferPtr ECDH::compute_secret(Buffer &public_key) const throw(const std::runtime_error &) {
  EC_POINT *pub = EC_POINT_new(group);
  int r = EC_POINT_oct2point(group, pub, public_key.data(), public_key.size(),
                             nullptr);
  if ((r == 0) || pub == nullptr)
    throw std::runtime_error("secret couldn't be computed with given key");

  // NOTE: field_size is in bits
  int field_size = EC_GROUP_get_degree(group);
  size_t size = ((size_t) field_size + 7) / 8;
  auto *out = new uint8_t[size];

  r = ECDH_compute_key(out, size, pub, key, nullptr);
  EC_POINT_free(pub);
  if (r == 0) {
    delete[] out;
    throw std::runtime_error("secret couldn't be computed with given key");
  }

  return std::move(make_intrusive_<Buffer>(out, size, size));
}

}  // namespace dsa

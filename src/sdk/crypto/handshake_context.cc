#include "dsa_common.h"

#include "handshake_context.h"

#include "hash.h"
#include "hmac.h"
#include "misc.h"

namespace dsa {

HandshakeContext::HandshakeContext(string_ dsid_prefix, ECDH &&ecdh)
    : _ecdh(std::move(ecdh)), _salt(32) {
  _dsid = _ecdh.get_dsid(dsid_prefix);
  //_salt = gen_salt(Connection::SALT_LENGTH);
  gen_salt(_salt.data(), _salt.size());
}
HandshakeContext::HandshakeContext(string_ dsid_prefix) : _salt(32) {
  Hash hash;
  hash.update(_ecdh.get_public_key());
  _dsid = dsid_prefix + hash.digest_base64();
  //_salt = gen_salt(Connection::SALT_LENGTH);
  gen_salt(_salt.data(), _salt.size());
}

void HandshakeContext::set_remote(string_ &&dsid,
                                  std::vector<uint8_t> &&public_key,
                                  std::vector<uint8_t> &&salt) {
  _remote_dsid = std::move(dsid);
  _remote_public_key = std::move(public_key);
  _remote_salt = std::move(salt);
}

void HandshakeContext::compute_secret() {
  _shared_secret = ecdh().compute_secret(_remote_public_key);

  /* compute user auth */
  HMAC hmac(_shared_secret);
  hmac.update(_remote_salt);
  _auth = hmac.digest();

  /* compute other auth */
  dsa::HMAC other_hmac(_shared_secret);
  other_hmac.update(salt());
  _remote_auth = other_hmac.digest();
}

}  // namespace dsa

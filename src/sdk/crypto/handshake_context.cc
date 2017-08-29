#include "dsa_common.h"

#include "handshake_context.h"

#include "hash.h"
#include "misc.h"
#include "hmac.h"

namespace dsa {

HandshakeContext::HandshakeContext(std::string dsid_prefix, const ECDH &ecdh)
    : _ecdh(ecdh) {
  _public_key = std::move(_ecdh.get_public_key());
  Hash hash("sha256");
  hash.update(_public_key);
  _dsid = dsid_prefix + base64url(hash.digest_base64());
  //_salt = gen_salt(Connection::SaltLength);
  _salt = gen_salt(32);
}
HandshakeContext::HandshakeContext(std::string dsid_prefix) {
  _public_key = std::move(_ecdh.get_public_key());
  Hash hash("sha256");
  hash.update(_public_key);
  _dsid = dsid_prefix + base64url(hash.digest_base64());
  //_salt = gen_salt(Connection::SaltLength);
  _salt = gen_salt(32);
}

void HandshakeContext::set_remote(std::string &&dsid,
                                  std::vector<uint8_t> &&public_key,
                                  std::vector<uint8_t> &&salt) {
  _remote_dsid = std::move(dsid);
}

void HandshakeContext::compute_secret() {
  _shared_secret = ecdh().compute_secret(_remote_public_key);

  /* compute user auth */
  HMAC hmac("sha256", _shared_secret);
  hmac.update(_remote_salt);
  _auth = hmac.digest();

  /* compute other auth */
  dsa::HMAC other_hmac("sha256", _shared_secret);
  other_hmac.update(salt());
  _remote_auth = other_hmac.digest();

#if DEBUG
  std::stringstream ss;
  ss << name() << "::compute_secret()" << std::endl;
  ss << "auth:       " << *_auth << std::endl;
  ss << "other auth: " << *_other_auth << std::endl;
  std::cout << ss.str();
#endif
}

}  // namespace dsa

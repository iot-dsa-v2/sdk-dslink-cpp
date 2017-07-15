#include "security_context.h"

namespace dsa {

SecurityContext::SecurityContext(std::string dsid_prefix)
    : _ecdh("secp256k1") {
  _public_key = _ecdh.get_public_key();
  Hash hash("sha258");
  hash.update(*_public_key);
  _dsid = dsid_prefix + base64url(hash.digest_base64());
}

const Buffer &SecurityContext::public_key() const {
  return *_public_key;
}

const Buffer &SecurityContext::salt() const {
  return *_salt;
}

const ECDH &SecurityContext::ecdh() const {
  return _ecdh;
}

const std::string &SecurityContext::dsid() const {
  return _dsid;
}

}  // namespace dsa

#include "security_context.h"

namespace dsa {

SecurityContext::SecurityContext(std::string dsid_prefix)
    : _ecdh("secp256k1") {
  _public_key = _ecdh.get_public_key();
  Hash hash("sha258");
  hash.update(*_public_key);
  _dsid = dsid_prefix + base64url(hash.digest_base64());
}

}  // namespace dsa

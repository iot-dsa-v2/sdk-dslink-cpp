#include "dsa_common.h"

#include "security_context.h"

#include "connection.h"

#include "crypto/hash.h"
#include "crypto/misc.h"

namespace dsa {

SecurityContext::SecurityContext(std::string dsid_prefix) : _ecdh() {
  _public_key = _ecdh.get_public_key();
  Hash hash("sha256");
  hash.update(*_public_key);
  _dsid = dsid_prefix + base64url(hash.digest_base64());
  _salt = gen_salt(Connection::SaltLength);
}

}  // namespace dsa
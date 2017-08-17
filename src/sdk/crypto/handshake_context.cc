#include "dsa_common.h"

#include "handshake_context.h"

#include "hash.h"
#include "misc.h"
#include "core/connection.h"

namespace dsa {

HandshakeContext::HandshakeContext(const std::string &dsid_prefix, 
								   const intrusive_ptr_<ECDH> &ecdh)
    : _ecdh(ecdh == nullptr ? new ECDH() : ecdh) {
  _public_key = _ecdh->get_public_key();
  Hash hash("sha256");
  hash.update(_public_key);
  _dsid = dsid_prefix + base64url(hash.digest_base64());
  _salt = gen_salt(Connection::SaltLength);
}

}  // namespace dsa

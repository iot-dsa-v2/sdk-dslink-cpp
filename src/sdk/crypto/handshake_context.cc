#include "dsa_common.h"

#include "handshake_context.h"

#include "hash.h"
#include "misc.h"

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

}  // namespace dsa

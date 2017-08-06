#ifndef DSA_SDK_HANDSHAKR_CONTEXT_H
#define DSA_SDK_HANDSHAKR_CONTEXT_H

#include <string>

#include "ecdh.h"


namespace dsa {

class HandshakeContext {
 private:
  BufferPtr _public_key;
  BufferPtr _salt;
  ECDH _ecdh;
  std::string _dsid;

 public:
  explicit HandshakeContext(std::string dsid_prefix);
  const Buffer &public_key() const { return *_public_key; };
  const Buffer &salt() const { return *_salt; };
  const ECDH &ecdh() const { return _ecdh; };
  std::string dsid() const { return _dsid; };
};

typedef shared_ptr_<HandshakeContext> SecurityContextPtr;

}  // namespace dsa

#endif  // DSA_SDK_HANDSHAKR_CONTEXT_H

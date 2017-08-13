#ifndef DSA_SDK_HANDSHAKR_CONTEXT_H
#define DSA_SDK_HANDSHAKR_CONTEXT_H

#include <string>

#include "ecdh.h"

namespace dsa {

class HandshakeContext {
 private:
  std::vector<uint8_t> _public_key;
  std::vector<uint8_t> _salt;
  ECDH _ecdh;
  std::string _dsid;

 public:
  explicit HandshakeContext(std::string dsid_prefix,
                            const ECDH *ecdh = nullptr);
  const std::vector<uint8_t> &public_key() const { return _public_key; };
  const std::vector<uint8_t> &salt() const { return _salt; };
  const ECDH &ecdh() const { return _ecdh; };
  const std::string &dsid() const { return _dsid; };
};

typedef shared_ptr_<HandshakeContext> SecurityContextPtr;

}  // namespace dsa

#endif  // DSA_SDK_HANDSHAKR_CONTEXT_H

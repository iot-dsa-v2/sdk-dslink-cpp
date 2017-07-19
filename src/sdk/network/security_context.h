#ifndef DSA_SDK_SECURITY_CONTEXT_H
#define DSA_SDK_SECURITY_CONTEXT_H

#include <string>

#include "dsa/util.h"
#include "dsa/crypto.h"

namespace dsa {

class SecurityContext {
 private:
  BufferPtr _public_key;
  BufferPtr _salt;
  ECDH _ecdh;
  std::string _dsid;

 public:
  SecurityContext(std::string dsid_prefix);
  const Buffer &public_key() const { return *_public_key; };
  const Buffer &salt() const { return *_salt; };
  const ECDH &ecdh() const { return _ecdh; };
  const std::string &dsid() const { return _dsid; };
};

typedef std::shared_ptr<SecurityContext> SecurityContextPtr;

}  // namespace dsa

#endif  // DSA_SDK_SECURITY_CONTEXT_H

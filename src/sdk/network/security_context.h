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
  const Buffer &public_key() const;
  const Buffer &salt() const;
  const ECDH &ecdh() const;
  const std::string &dsid() const;
};

typedef std::shared_ptr<SecurityContext> SecurityContextPtr;

}  // namespace dsa

#endif  // DSA_SDK_SECURITY_CONTEXT_H

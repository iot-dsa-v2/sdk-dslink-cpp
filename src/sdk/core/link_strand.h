#ifndef DSA_SDK_LINK_STRAND_H
#define DSA_SDK_LINK_STRAND_H

#include "util/enable_intrusive.h"

#include <boost/asio/strand.hpp>

namespace dsa {

class SecurityManager;
class SessionManager;
class NodeStateManager;
class ECDH;

typedef boost::asio::io_service::strand Strand;

class LinkStrand : public EnableIntrusive<LinkStrand> {
 private:
  // managed pointer by LinkStrand
  Strand *__strand{nullptr};
  ECDH *__ecdh{nullptr};

 protected:
  SecurityManager *__security_manager{nullptr};
  NodeStateManager *__state_manager{nullptr};
  SessionManager *__session_manager{nullptr};

 public:
  explicit LinkStrand(Strand *strand, ECDH *ecdh);

  virtual ~LinkStrand();

  Strand &operator()() { return *__strand; }

  SecurityManager &security_manager() { return *__security_manager; };

  NodeStateManager &state_manager() { return *__state_manager; };

  SessionManager &session_manager() { return *__session_manager; };

  ECDH &ecdh() { return *__ecdh; };
};
typedef intrusive_ptr_<LinkStrand> LinkStrandPtr;
}  // namespace dsa
#endif  // DSA_SDK_LINK_STRAND_H
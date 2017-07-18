#include "server.h"

namespace dsa {

Server::Server(SecurityContext &security_context) : _security_context(security_context) {}

const SecurityContext &Server::security_context() {
  return _security_context;
}

}  // namespace dsa
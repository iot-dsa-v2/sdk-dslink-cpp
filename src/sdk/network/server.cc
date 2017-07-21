#include "server.h"

#include "session.h"

namespace dsa {

void Server::stop() {
  for (auto& kv : _sessions) {
    kv.second.reset();
  }
}

}  // namespace dsa
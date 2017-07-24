#include "server.h"

#include "session.h"

namespace dsa {

void Server::stop() {
  for (auto& kv : _sessions) {
    if (kv.second != nullptr) {
      kv.second->stop();
      kv.second.reset();
    }
  }
}

}  // namespace dsa
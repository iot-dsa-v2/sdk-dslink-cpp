#include <module/default/simple_security_manager.h>

#include "client.h"

namespace dsa {
Client::Client(WrapperConfig & config)
    : _strand(config.strand),
      _client_token(config.client_token),
      _session( make_intrusive_<Session>(config.strand, "", nullptr)) {}

void Client::close() {
  _connection->close();
  Closable::close();
}
}  // namespace dsa

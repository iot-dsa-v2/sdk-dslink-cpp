#include <module/default/simple_security_manager.h>
#include "dsa_common.h"

#include "client.h"
#include "network/client_connection.h"

namespace dsa {
Client::Client(WrapperConfig & config)
    : _strand(config.strand),
      _client_token(config.client_token),
      _session( make_intrusive_<Session>(config.strand, "", nullptr)) {}

void Client::close() {
  _connection->close();
}
}  // namespace dsa

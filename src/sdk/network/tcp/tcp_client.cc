#include "dsa_common.h"

#include "tcp_client.h"
#include "tcp_client_connection.h"
#include "network/client_connection.h"

namespace dsa {
TcpClient::TcpClient(const Config &config)
    : Client(config), 
      _hostname(config.tcp_host), 
      _port(config.tcp_port) {
  _connection = make_shared_<TcpClientConnection>(config);
  _session = make_intrusive_<Session>(*this, "", _connection);
  _connection->set_session(_session);
}

void TcpClient::connect() {
  _connection->connect();
}
}  // namespace dsa

#include "dsa_common.h"

#include "network/client_connection.h"
#include "tcp_client.h"
#include "tcp_client_connection.h"

namespace dsa {
TcpClient::TcpClient(WrapperConfig &config)
    : Client(config), _hostname(config.tcp_host), _port(config.tcp_port) {
  _connection = make_shared_<TcpClientConnection>(
      config.strand, config.handshake_timeout_ms, config.dsid_prefix,
      config.tcp_host, config.tcp_port);
  _session = make_intrusive_<Session>(config.strand, "", _connection);
  _connection->set_session(_session);
}

void TcpClient::connect() { _connection->connect(); }
}  // namespace dsa

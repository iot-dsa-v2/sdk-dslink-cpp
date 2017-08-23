//
// Created by Ben Richards on 8/9/17.
//

#ifndef DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_

#include "../connection.h"
#include "tcp_connection.h"
#include "network/server_connection.h"
#include "util/enable_shared.h"

namespace dsa {
class TcpServer;

// TCP server side connection.
// Handles server side of DSA handshake and starts read loop.
class TcpServerConnection : public TcpConnection, public ServerConnection {
 private:
  void f0_received(const boost::system::error_code &error,
                   size_t bytes_transferred);
  void f2_received(const boost::system::error_code &error,
                   size_t bytes_transferred);
  void send_f3();

 protected:
  void start_handshake();

 public:
  TcpServerConnection(LinkStrandPtr strand, uint32_t handshake_timeout_ms,
                      const std::string &dsid_prefix = "",
                      const std::string &path = "");


  ~TcpServerConnection() { std::cout << "~TcpServerConnection()\n"; }

  void connect() override;

  std::string name() override { return "TcpServerConnection"; }

  void set_server(shared_ptr_<TcpServer> server) noexcept {
    _server = std::move(std::dynamic_pointer_cast<Server>(server));
  };
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_

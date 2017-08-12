//
// Created by Ben Richards on 8/9/17.
//

#ifndef DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_

#include "core/connection.h"
#include "tcp_connection.h"
#include "../server_connection.h"
#include "util/enable_shared.h"

namespace dsa {
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
  TcpServerConnection(boost::asio::io_service::strand &strand, const Config &config);
  ~TcpServerConnection() { std::cout << "~TcpServerConnection()\n"; }

  void connect() override;

  std::string name() override { return "TcpServerConnection"; }

  void set_server(shared_ptr_<TcpServer> server) noexcept {
    _server = std::move(server);
  };
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_

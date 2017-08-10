//
// Created by Ben Richards on 8/9/17.
//

#ifndef DSA_SDK_NETWORK_CONNECTION_TCP_CLIENT_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_TCP_CLIENT_CONNECTION_H_

#include "core/connection.h"
#include "tcp_connection.h"
#include "client_connection.h"
#include "util/enable_shared.h"

namespace dsa {
// TCP client side connection.
// Handles client side of DSA handshake and starts read loop.
class TcpClientConnection : public TcpConnection, public ClientConnection {
 private:
  void f1_received(const boost::system::error_code &error,
                   size_t bytes_transferred);
  void f3_received(const boost::system::error_code &error,
                   size_t bytes_transferred);

 protected:
  void start_handshake(const boost::system::error_code &error) throw(const std::runtime_error &);

 public:
  TcpClientConnection(boost::asio::io_service::strand &strand, const Config &config, intrusive_ptr_<Session> session);
  ~TcpClientConnection() { std::cout << "~TcpClientConnection()\n"; }

  std::string name() override { return "TcpClientConnection"; }

  void connect() override;

  intrusive_ptr_<Session> session() { return _session; }
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_TCP_CLIENT_CONNECTION_H_

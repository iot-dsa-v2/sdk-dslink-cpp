#ifndef DSA_SDK_NETWORK_CONNECTION_TCP_CLIENT_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_TCP_CLIENT_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../connection.h"
#include "tcp_connection.h"
#include "util/enable_shared.h"

namespace dsa {
class TcpClient;

// TCP client side connection.
// Handles client side of DSA handshake and starts read loop.
class TcpClientConnection : public TcpConnection {
 private:

  void f3_received(const boost::system::error_code &error,
                   size_t bytes_transferred);

 protected:
  std::string _hostname;
  uint16_t _port;


 public:
  TcpClientConnection(LinkStrandRef & strand, uint32_t handshake_timeout_ms,
                      const std::string &dsid_prefix,
                      const std::string &tcp_host, uint16_t tcp_port,
                      const std::string &path = "");

  std::string name() override { return "TcpClientConnection"; }

  void connect() override;

  ref_<Session> session() { return _session; }
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_TCP_CLIENT_CONNECTION_H_

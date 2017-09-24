#ifndef DSA_SDK_NETWORK_CONNECTION_TCP_CLIENT_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_TCP_CLIENT_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../connection.h"
#include "tcp_connection.h"
#include "util/enable_shared.h"

namespace dsa {
class Client;

// TCP client side connection.
// Handles client side of DSA handshake and starts read loop.
class ClientConnection : public TcpConnection {
 private:

 protected:
  std::string _hostname;
  uint16_t _port;


 public:
  ClientConnection(LinkStrandRef & strand,
                      const std::string &dsid_prefix,
                      const std::string &tcp_host, uint16_t tcp_port,
                      uint32_t handshake_timeout_ms = 5000);

  std::string name() override { return "ClientConnection"; }

  void connect() override;

};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_TCP_CLIENT_CONNECTION_H_

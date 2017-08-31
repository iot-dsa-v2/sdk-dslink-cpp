//
// Created by Ben Richards on 8/9/17.
//

#ifndef DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_

#include "../connection.h"
#include "tcp_connection.h"
#include "util/enable_shared.h"

namespace dsa {
class TcpServer;

// TCP server side connection.
// Handles server side of DSA handshake and starts read loop.
class TcpServerConnection : public TcpConnection {
 public:
  TcpServerConnection(LinkStrandRef &strand, uint32_t handshake_timeout_ms,
                      const std::string &dsid_prefix = "",
                      const std::string &path = "");

  ~TcpServerConnection() { std::cout << "~TcpServerConnection()\n"; }

  void accept() override;

  std::string name() override { return "TcpServerConnection"; }
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_

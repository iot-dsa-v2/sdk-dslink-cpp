#ifndef DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

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
                      const string_ &dsid_prefix = "",
                      const string_ &path = "");

  void accept() override;

  string_ name() override { return "TcpServerConnection"; }
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H_

#ifndef DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H
#define DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H

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
class TcpServerConnection final : public TcpConnection {
 public:
  TcpServerConnection(const LinkStrandRef &strand, const string_ &dsid_prefix = "",
                      const string_ &path = "");

  void accept() final;

  string_ name() final { return "TcpServerConnection"; }
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_TCP_SERVER_CONNECTION_H

#ifndef DSA_SDK_NETWORK_WS_CLIENT_CONNECTION_H
#define DSA_SDK_NETWORK_WS_CLIENT_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../connection.h"
#include "util/enable_shared.h"
#include "ws_connection.h"

#include <mutex>

namespace dsa {

// WS client side connection.
// Handles client side of DSA handshake and starts read loop.
class WsClientConnection final : public WsConnection {
 private:
  tcp::socket _tcp_socket;
  boost::asio::ssl::context _ssl_context;

 protected:
  string_ _hostname;
  uint16_t _port;

 public:
  WsClientConnection(bool is_secured, const SharedLinkStrandRef &strand,
                     const string_ &dsid_prefix, const string_ &ws_host,
                     uint16_t ws_port);

  string_ name() final { return "WsClientConnection"; }

  void connect(size_t reconnect_interval) final;
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_WS_CLIENT_CONNECTION_H

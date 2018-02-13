#ifndef DSA_SDK_NETWORK_WSS_CLIENT_CONNECTION_H_
#define DSA_SDK_NETWORK_WSS_CLIENT_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../connection.h"
#include "util/enable_shared.h"
#include "wss_connection.h"

namespace dsa {

// WS client side connection.
// Handles client side of DSA handshake and starts read loop.
class WssClientConnection final : public WssConnection {
 private:
 protected:
  string_ _hostname;
  uint16_t _port;

 public:
  WssClientConnection(websocket_ssl_stream &stream, LinkStrandRef &strand,
                      const string_ &dsid_prefix, const string_ &ws_host,
                      uint16_t ws_port);

  string_ name() final { return "WssClientConnection"; }

  void connect(size_t reconnect_interval) final;
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_WSS_CLIENT_CONNECTION_H_

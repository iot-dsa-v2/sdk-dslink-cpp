#ifndef DSA_SDK_NETWORK_WSS_CLIENT_CONNECTION_H_
#define DSA_SDK_NETWORK_WSS_CLIENT_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../connection.h"
#include "util/enable_shared.h"
#include "wss_connection.h"

#include <mutex>

namespace dsa {

// WS client side connection.
// Handles client side of DSA handshake and starts read loop.
class WssClientConnection final : public WssConnection {
 private:
  tcp::socket _tcp_socket;
  boost::asio::ssl::context _ssl_context;
  std::unique_ptr<websocket_ssl_stream> _wss_stream;

 protected:
  string_ _hostname;
  uint16_t _port;
  std::mutex _mutex;

 public:
  WssClientConnection(LinkStrandRef &strand, const string_ &dsid_prefix,
                      const string_ &ws_host, uint16_t ws_port);

  websocket_ssl_stream &secure_stream() { return *_wss_stream; }

  string_ name() final { return "WssClientConnection"; }

  void connect(size_t reconnect_interval) final;
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_WSS_CLIENT_CONNECTION_H_

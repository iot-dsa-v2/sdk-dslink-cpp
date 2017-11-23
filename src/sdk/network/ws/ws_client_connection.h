#ifndef DSA_SDK_NETWORK_CONNECTION_WS_CLIENT_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_WS_CLIENT_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../connection.h"
#include "util/enable_shared.h"
#include "ws_connection.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;  // from <boost/asio/ip/tcp.hpp>
namespace websocket =
    boost::beast::websocket;  // from <boost/beast/websocket.hpp>

typedef websocket::stream<tcp::socket> websocket_stream;

// WS client side connection.
// Handles client side of DSA handshake and starts read loop.
class WsClientConnection final : public WsConnection {
 private:
 protected:
  string_ _hostname;
  uint16_t _port;

 public:
  WsClientConnection(LinkStrandRef &strand, const string_ &dsid_prefix,
                     const string_ &ws_host, uint16_t ws_port);

  string_ name() final { return "WsClientConnection"; }

  void connect(size_t reconnect_interval) final;
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_WS_CLIENT_CONNECTION_H_

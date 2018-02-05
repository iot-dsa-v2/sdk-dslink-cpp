#ifndef DSA_SDK_NETWORK_WS_SERVER_CONNECTION_H
#define DSA_SDK_NETWORK_WS_SERVER_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/beast/core/flat_buffer.hpp>

#include "../connection.h"
#include "ws_connection.h"

namespace http = boost::beast::http;  // from <boost/beast/http.hpp>

namespace dsa {

// Web server side connection.
// Handles server side of DSA handshake and starts read loop.
class WsServerConnection final : public WsConnection {
 private:
  boost::beast::flat_buffer _buffer;
  http::request<http::string_body> _req;

 public:
  WsServerConnection(websocket_stream &ws, LinkStrandRef &strand,
                     const string_ &dsid_prefix = "", const string_ &path = "");

  void accept() final;

  string_ name() final { return "WsServerConnection"; }
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_WS_SERVER_CONNECTION_H

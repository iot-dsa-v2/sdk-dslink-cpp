#ifndef DSA_SDK_NETWORK_WSS_SERVER_CONNECTION_H_
#define DSA_SDK_NETWORK_WSS_SERVER_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/beast/core/flat_buffer.hpp>

#include "../../web_server/websocket.h"
#include "../connection.h"
#include "wss_connection.h"

namespace http = boost::beast::http;  // from <boost/beast/http.hpp>

namespace dsa {

// Web server side connection.
// Handles server side of DSA handshake and starts read loop.
class WssServerConnection final : public WssConnection {
 private:
  boost::beast::flat_buffer _buffer;
  http::request<http::string_body> _req;

  std::unique_ptr<Websocket> _websocket;

 public:
  WssServerConnection(std::unique_ptr<Websocket> websocket,
                      LinkStrandRef &strand, const string_ &dsid_prefix = "",
                      const string_ &path = "");

  websocket_ssl_stream &secure_stream() final {
    return _websocket->secure_stream();
  }

  void accept() final;

  string_ name() final { return "WssServerConnection"; }
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_WSS_SERVER_CONNECTION_H_

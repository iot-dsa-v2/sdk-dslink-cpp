#ifndef DSA_SDK_NETWORK_WSS_SERVER_CONNECTION_H_
#define DSA_SDK_NETWORK_WSS_SERVER_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/beast/core/flat_buffer.hpp>

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

 public:
  WssServerConnection(websocket_ssl_stream &, LinkStrandRef &strand,
                      const string_ &dsid_prefix = "",
                      const string_ &path = "");

  void accept() final;

  string_ name() final { return "WssServerConnection"; }
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_WSS_SERVER_CONNECTION_H_

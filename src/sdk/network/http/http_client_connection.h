#ifndef DSA_SDK_NETWORK_CONNECTION_HTTP_CLIENT_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_HTTP_CLIENT_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../connection.h"
#include "http_connection.h"
#include "util/enable_shared.h"

namespace dsa {

// HTTP client side connection.
// Handles client side of DSA handshake and starts read loop.
class HttpClientConnection final : public HttpConnection {
 private:
 protected:
  string_ _hostname;
  uint16_t _port;

 public:
  HttpClientConnection(LinkStrandRef &strand, const string_ &dsid_prefix,
                   const string_ &http_host, uint16_t http_port);

  string_ name() final { return "HttpClientConnection"; }

  void connect(size_t reconnect_interval) final;
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_HTTP_CLIENT_CONNECTION_H_

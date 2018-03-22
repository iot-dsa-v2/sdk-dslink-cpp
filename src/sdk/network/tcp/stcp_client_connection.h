#ifndef DSA_SDK_STCP_CLIENT_CONNECTION_H
#define DSA_SDK_STCP_CLIENT_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../connection.h"
#include "stcp_connection.h"
#include "util/enable_shared.h"

namespace dsa {
class Client;

// TCP client side connection.
// Handles client side of DSA handshake and starts read loop.
class StcpClientConnection final : public StcpConnection {
 private:
 protected:
  string_ _hostname;
  uint16_t _port;

 public:
  StcpClientConnection(const LinkStrandRef &strand,
                       boost::asio::ssl::context &context,
                       const string_ &dsid_prefix, const string_ &tcp_host,
                       uint16_t tcp_port);

  bool verify_certificate(bool preverified,
                          boost::asio::ssl::verify_context &context);

  string_ name() final { return "StcpClientConnection"; }

  void connect(size_t reconnect_interval) final;
};

}  // namespace dsa

#endif  // DSA_SDK_STCP_CLIENT_CONNECTION_H

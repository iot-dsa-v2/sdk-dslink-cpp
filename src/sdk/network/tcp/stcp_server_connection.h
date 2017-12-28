#ifndef DSA_SDK_STCP_SERVER_CONNECTION_H_
#define DSA_SDK_STCP_SERVER_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ssl.hpp>

#include "../connection.h"
#include "stcp_connection.h"
#include "util/enable_shared.h"

namespace dsa {

// TCP server side connection.
// Handles server side of DSA handshake and starts read loop.
class StcpServerConnection final : public StcpConnection {
 public:
  StcpServerConnection(LinkStrandRef &strand,
                       boost::asio::ssl::context &_context,
                       const string_ &dsid_prefix = "",
                       const string_ &path = "");

  void accept() final;

  string_ name() final { return "StcpServerConnection"; }
};
}  // namespace dsa

#endif  // DSA_SDK_STCP_SERVER_CONNECTION_H_

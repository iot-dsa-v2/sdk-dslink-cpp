#ifndef DSA_SDK_NETWORK_CONNECTION_WS_SERVER_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_WS_SERVER_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../connection.h"
#include "ws_connection.h"
#include "util/enable_shared.h"

namespace dsa {

// Web server side connection.
// Handles server side of DSA handshake and starts read loop.
class WsServerConnection final : public WsConnection {
 public:
  WsServerConnection(LinkStrandRef &strand, const string_ &dsid_prefix = "",
                      const string_ &path = "");

  void accept() final;

  string_ name() final { return "WsServerConnection"; }
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_WS_SERVER_CONNECTION_H_

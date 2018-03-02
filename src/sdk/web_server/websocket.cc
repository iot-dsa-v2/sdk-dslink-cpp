#include "dsa_common.h"

#include "websocket.h"

namespace dsa {

Websocket::Websocket(tcp::socket&& socket)
    : _is_secure_stream(false),
      _socket(std::move(socket)),
      _ws_stream(std::make_unique<websocket_stream>(std::move(_socket))) {}

Websocket::Websocket(tcp::socket&& socket, ssl::context& ssl_context)
    : _is_secure_stream(true),
      _socket(std::move(socket)),
      _wss_stream(
          std::make_unique<websocket_ssl_stream>(_socket, ssl_context)) {}

}  // namespace dsa

#include "dsa_common.h"

#include "websocket.h"

namespace dsa {

Websocket::Websocket(tcp::socket&& socket, ssl::context& ssl_context)
    : _is_secure_stream(true),
      _socket(std::move(socket)),
      _wss_stream(_socket, ssl_context) {}

}  // namespace dsa

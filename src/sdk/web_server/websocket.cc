#include "dsa_common.h"

#include "websocket.h"

namespace dsa {

Websocket::Websocket(boost::asio::io_context& io_context,
                     ssl::context& ssl_context)
    : _is_secure_stream(false),
      _socket(io_context),
      _wss_stream(_socket, ssl_context) {}

}  // namespace dsa

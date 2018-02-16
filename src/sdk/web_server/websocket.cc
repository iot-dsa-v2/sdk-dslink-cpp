#include "dsa_common.h"

#include "websocket.h"

namespace dsa {

Websocket::Websocket(boost::asio::io_context& io_context)
    : _is_secure_stream(false),
      _context(boost::asio::ssl::context::sslv23),
      _socket(io_context),
      _wss_stream(_socket, _context) {}

}  // namespace dsa

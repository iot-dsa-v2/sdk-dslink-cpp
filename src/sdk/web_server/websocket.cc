#include "dsa_common.h"

#include "websocket.h"

namespace dsa {

Websocket::Websocket(boost::asio::io_context& io_context)
    : _is_secured_stream(false),
      _context(boost::asio::ssl::context::sslv23),
      _socket(io_context),
      _wss_stream(*std::make_shared<websocket_ssl_stream>(_socket, _context)) {}

Websocket::Websocket(tcp::socket socket)
    : _is_secured_stream(false),
      _context(boost::asio::ssl::context::sslv23),
      _socket(std::move(socket)),
      _wss_stream(*std::make_shared<websocket_ssl_stream>(_socket, _context)) {}

Websocket::Websocket(websocket_ssl_stream& wss_stream)
    : _is_secured_stream(true),
      _context(boost::asio::ssl::context::sslv23),
      _socket(wss_stream.get_executor().context()),
      _wss_stream(wss_stream) {}
}  // namespace dsa

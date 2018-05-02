#include "dsa_common.h"

#include "websocket.h"

#include "module/logger.h"

namespace dsa {

Websocket::Websocket(tcp::socket&& socket)
    : _is_secure_stream(false),
      _is_websocket(false),
      _socket(std::move(socket)),
      _ws_stream(std::make_unique<websocket_stream>(std::move(_socket))) {}

Websocket::Websocket(tcp::socket&& socket, ssl::context& ssl_context)
    : _is_secure_stream(true),
      _is_websocket(false),
      _socket(std::move(socket)),
      _wss_stream(
          std::make_unique<websocket_ssl_stream>(_socket, ssl_context)) {}

void Websocket::destroy_impl() {
  try {
    if (_is_secure_stream) {
      // server and client close differently
      _is_websocket
          ? _wss_stream->lowest_layer().close()
          : _wss_stream->close(websocket::close_code::normal);
    } else {
      _ws_stream->next_layer().close();
    }
  } catch (boost::system::system_error& e) {
    LOG_DEBUG(__FILENAME__, LOG << "websocket close error: " << e.what());
  }
}

}  // namespace dsa

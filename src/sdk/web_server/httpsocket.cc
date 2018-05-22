#include "dsa_common.h"

#include "socket.h"

#include <boost/asio/connect.hpp>
#include "module/logger.h"

namespace dsa {

void Websocket::http_async_read(
    boost::beast::flat_buffer& buffer,
    boost::beast::http::request<boost::beast::http::string_body>& req,
    Callback&& handler) {
  if (_is_websocket) {
    LOG_FATAL("Websocket",
              LOG << "can not access http on websocket connection");
  }
  if (_is_secure_stream) {
    _wss_stream->next_layer().async_handshake(ssl::stream_base::server, [
      this, &buffer, &req, handler = std::move(handler)
    ](const boost::system::error_code& error) mutable {
      if (is_destroyed()) return;
      if (error != boost::system::errc::success) {
        handler(error, 0);
        return;
      }
      boost::beast::http::async_read(_wss_stream->next_layer(), buffer, req,
                                     std::move(handler));
    });
  } else {
    // Read a request
    boost::beast::http::async_read(_ws_stream->next_layer(), buffer, req,
                                   std::move(handler));
  }
}
void Websocket::http_async_write(http::response<http::string_body>& res,
                                 Callback&& handler) {
  if (_is_websocket) {
    LOG_FATAL("Websocket",
              LOG << "can not access http on websocket connection");
  }
  if (_is_secure_stream) {
    http::async_write(_wss_stream->next_layer(), res, std::move(handler));
  } else {
    http::async_write(_ws_stream->next_layer(), res, std::move(handler));
  }
  destroy();
}

}  // namespace dsa

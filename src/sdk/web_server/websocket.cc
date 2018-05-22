#include "dsa_common.h"

#include "socket.h"

#include <boost/asio/connect.hpp>
#include "module/logger.h"

namespace dsa {

void Websocket::async_accept(
    boost::beast::http::request<boost::beast::http::string_body>& req,
    ConnectCallback&& handler) {
  if (_is_websocket) {
    LOG_FATAL("Websocket", LOG << "async_accept called on accepted connection");
  }
  if (is_secure_stream()) {
    _wss_stream->async_accept(std::move(handler));
  } else {
    _ws_stream->async_accept(std::move(handler));
  }
}

void Websocket::async_read_some(boost::asio::mutable_buffer&& buffer,
                                Callback&& handler) {
  if (!_is_websocket) {
    LOG_FATAL("Websocket", LOG << "not websocket");
  }
  if (is_secure_stream()) {
    std::lock_guard<std::mutex> lock(_mutex);
    _wss_stream->async_read_some(std::move(buffer), std::move(handler));
  } else {
    _ws_stream->async_read_some(std::move(buffer), std::move(handler));
  }
}

void Websocket::async_write(boost::asio::mutable_buffer&& buffer,
                            Callback&& handler) {
  if (!_is_websocket) {
    LOG_FATAL("Websocket", LOG << "not websocket");
  }
  if (is_secure_stream()) {
    std::lock_guard<std::mutex> lock(_mutex);
    _wss_stream->async_write(std::move(buffer), std::move(handler));
  } else {
    _ws_stream->async_write(std::move(buffer), std::move(handler));
  }
}

}  // namespace dsa

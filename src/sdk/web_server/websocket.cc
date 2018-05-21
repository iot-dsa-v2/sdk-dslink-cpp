#include "dsa_common.h"

#include "websocket.h"

#include <boost/asio/connect.hpp>
#include "module/logger.h"

namespace dsa {

Websocket::Websocket(tcp::socket&& socket)
    : _is_secure_stream(false),
      _socket(std::move(socket)),
      _ws_stream(std::make_unique<websocket_stream>(std::move(_socket))) {
  _ws_stream->binary(true);
}

Websocket::Websocket(tcp::socket&& socket, ssl::context& ssl_context)
    : _is_secure_stream(true),
      _socket(std::move(socket)),
      _wss_stream(
          std::make_unique<websocket_ssl_stream>(_socket, ssl_context)) {
  _wss_stream->binary(true);
}

Websocket::~Websocket() = default;

void Websocket::async_connect(const string_& hostname,
                              tcp::resolver::results_type& resolved,
                              ConnectCallback&& handler) {
  _is_websocket = true;
  if (!is_secure_stream()) {
    // TODO: timeout

    boost::asio::async_connect(
        _ws_stream->next_layer(), resolved.begin(), resolved.end(),
        // capture shared_ptr to keep the instance
        // capture this to access protected member
        [ this, hostname, handler = std::move(handler) ](
            const boost::system::error_code& error,
            tcp::resolver::iterator) mutable {
          if (is_destroyed()) return;
          if (error != boost::system::errc::success) {
            handler(error);
            return;
          }
          // websocket handshake
          _ws_stream->async_handshake(hostname, "/", std::move(handler));
        });
  } else {
    std::lock_guard<std::mutex> lock(_mutex);
    boost::asio::async_connect(
        _wss_stream->next_layer().next_layer(), resolved.begin(),
        resolved.end(),
        // capture shared_ptr to keep the instance
        // capture this to access protected member
        [ this, hostname, handler = std::move(handler) ](
            const boost::system::error_code& error,
            tcp::resolver::iterator) mutable {
          if (is_destroyed()) return;
          if (error != boost::system::errc::success) {
            handler(error);
            return;
          }
          std::lock_guard<std::mutex> lock(_mutex);
          // ssl handshake
          _wss_stream->next_layer().async_handshake(ssl::stream_base::client, [
            this, hostname, handler = std::move(handler)
          ](boost::system::error_code ec) {
            if (is_destroyed()) return;
            if (ec != boost::system::errc::success) {
              handler(ec);
              return;
            }

            std::lock_guard<std::mutex> lock(_mutex);
            // websocket handshake
            _wss_stream->async_handshake(
                hostname, "/", std::move(handler));  // websocket handshake

          });  // ssl handshake

        });  // async_connect
  }
}

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

void Websocket::http_async_read(
    boost::beast::flat_buffer buffer,
    boost::beast::http::request<boost::beast::http::string_body>& req,
    Callback&& handler) {
  if (_is_websocket) {
    LOG_FATAL("Websocket",
              LOG << "can not access http on websocket connection");
  }
  if (_is_secure_stream) {
    _wss_stream->next_layer().async_handshake(ssl::stream_base::server, [
      this, buffer, &req, handler = std::move(handler)
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
void Websocket::http_async_write(boost::asio::mutable_buffer&& buffer,
                                 Callback&& handler) {
  if (_is_websocket) {
    LOG_FATAL("Websocket",
              LOG << "can not access http on websocket connection");
  }
}

void Websocket::destroy() {
  if (_destroyed) return;

  _destroyed = true;
  auto on_close = [](const boost::system::error_code ec) {
    if (ec) {
      LOG_DEBUG(__FILENAME__, LOG << "websocket close error: " << ec.message());
    }
  };
  if (_is_websocket) {
    // destroy websocket
    if (is_secure_stream()) {
      std::lock_guard<std::mutex> lock(_mutex);
      _wss_stream->async_close(websocket::close_code::normal, on_close);
    } else {
      _ws_stream->async_close(websocket::close_code::normal, on_close);
    }
  } else {
    // destroy http
  }
}
}  // namespace dsa

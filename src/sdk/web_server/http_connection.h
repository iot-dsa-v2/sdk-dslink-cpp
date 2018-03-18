#ifndef DSA_SDK_HTTP_CONNECTION_H
#define DSA_SDK_HTTP_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include "dsa_common.h"

#include "detect_ssl.h"
#include "http_request.h"
#include "ssl_stream.h"
#include "web_server.h"
#include "websocket.h"

#include <memory>
#include "fields_alloc.h"

namespace http = boost::beast::http;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

namespace {

void fail(boost::system::error_code ec, char const* what) {
  std::cerr << what << ": " << ec.message() << "\n";
}
}

namespace dsa {

template <typename InternalType>
struct Self {
  InternalType& internal_http_type() {
    return static_cast<InternalType&>(*this);
  }
  InternalType const& internal_http_type() const {
    return static_cast<InternalType const&>(*this);
  }
};

class WebServer;
class HttpRequest;
class PlainHttpRequest;
class SecureHttpRequest;
class PlainHttpConnection;
class SecureHttpConnection;

template <typename HttpType>
class HttpConnection : public Self<HttpType> {
 protected:
  using alloc_t = fields_alloc<char>;
  using request_body_t =
      http::basic_dynamic_body<boost::beast::flat_static_buffer<1024 * 1024>>;
  WebServer& _web_server;
  shared_ptr_<Websocket> _websocket;
  alloc_t _alloc = alloc_t{8192};
  boost::beast::flat_buffer _buffer{8192};
  boost::optional<http::request_parser<request_body_t, alloc_t>> _parser;
  shared_ptr_<HttpRequest> _req;
  //  boost::asio::basic_waitable_timer<std::chrono::steady_clock> _deadline;
  std::mutex _mutex;

 public:
  //  void check_deadline();
  auto request() { return _req; }

 public:
  HttpConnection(WebServer& web_server, boost::beast::flat_buffer buffer)
      : _web_server(web_server), _buffer(std::move(buffer)) {}

  virtual ~HttpConnection() { destroy(); }

  void reset_parser() {
    _parser.emplace(std::piecewise_construct, std::make_tuple(),
                    std::make_tuple(_alloc));
  }

  void destroy() { _alloc.pool_.destroy(); }

  void upgrade_to_websocket() {
    // TODO
  }

  void create_request() { this->internal_http_type()._create_request(); }

  void do_read() {
    reset_parser();
    http::async_read(this->internal_http_type().stream(), _buffer, *_parser, [
      this, sthis = this->internal_http_type().shared_from_this()
    ](const boost::system::error_code& ec, size_t bytes_transferred) {
      std::lock_guard<std::mutex> lock(_mutex);

      if (ec == boost::asio::error::operation_aborted) return;

      // Client closed the connection
      if (ec == http::error::end_of_stream)
        return this->internal_http_type().do_eof();

      if (ec) return fail(ec, "read");

      if (_parser->upgrade()) {
        upgrade_to_websocket();
      }
      auto _target = _parser->get().target().to_string();
      create_request();
      _web_server.http_handler(_target)(_web_server, std::move(*_req));
      return;
    });
  }
};

class PlainHttpConnection
    : public HttpConnection<PlainHttpConnection>,
      public std::enable_shared_from_this<PlainHttpConnection> {
 private:
  tcp::socket _socket;

 public:
  explicit PlainHttpConnection(tcp::socket socket, WebServer& webserver,
                               boost::beast::flat_buffer buffer)
      : HttpConnection<PlainHttpConnection>(webserver, std::move(buffer)),
        _socket(std::move(socket)) {}

  ~PlainHttpConnection() override = default;

  tcp::socket& stream() { return _socket; }

  tcp::socket release_stream() { return std::move(_socket); }

  void _create_request() {
    _req = std::static_pointer_cast<HttpRequest>(
        std::make_shared<PlainHttpRequest>(_web_server, release_stream(),
                                           std::move(_parser->get())));
  }

  void run() { do_read(); }
  void do_eof() {
    boost::system::error_code ec;
    _socket.shutdown(tcp::socket::shutdown_send, ec);
  }
};

class SecureHttpConnection
    : public HttpConnection<SecureHttpConnection>,
      public std::enable_shared_from_this<SecureHttpConnection> {
 private:
  ssl_stream<tcp::socket> _stream;
  bool eof_ = false;

 public:
  explicit SecureHttpConnection(tcp::socket socket, ssl::context& ctx,
                                WebServer& webserver,
                                boost::beast::flat_buffer buffer)
      : HttpConnection<SecureHttpConnection>(webserver, std::move(buffer)),
        _stream(std::move(socket), ctx) {}

  ~SecureHttpConnection() override = default;

  ssl_stream<tcp::socket>& stream() { return _stream; }

  ssl_stream<tcp::socket> release_stream() { return std::move(_stream); }

  void _create_request() {
    _req = std::static_pointer_cast<HttpRequest>(
        std::make_shared<SecureHttpRequest>(_web_server, release_stream(),
                                            std::move(_parser->get())));
  }

  void run() {
    _stream.async_handshake(ssl::stream_base::server, _buffer.data(), [
      this, self = shared_from_this()
    ](boost::system::error_code ec, std::size_t bytes_used) {
      if (ec == boost::asio::error::operation_aborted) return;

      if (ec) return fail(ec, "handshake");

      // Consume the portion of the buffer used by the handshake
      _buffer.consume(bytes_used);

      do_read();
    });
  }

  void do_eof() {
    eof_ = true;

    _stream.async_shutdown([](boost::system::error_code ec) {
      if (ec == boost::asio::error::operation_aborted) return;

      if (ec) return fail(ec, "shutdown");
    });
  }
};

class DetectConnection : public std::enable_shared_from_this<DetectConnection> {
  tcp::socket socket_;
  ssl::context& ctx_;
  boost::beast::flat_buffer buffer_;
  WebServer& _web_server;
  std::mutex _mutex;

 public:
  DetectConnection(tcp::socket socket, ssl::context& ctx, WebServer& webserver)
      : socket_(std::move(socket)), ctx_(ctx), _web_server(webserver) {}

  void run() {
    async_detect_ssl(socket_, buffer_, [
      this, sthis = shared_from_this()
    ](const boost::system::error_code& ec, boost::tribool result) {
      std::lock_guard<std::mutex> lock(_mutex);
      if (ec) return fail(ec, "detect");

      if (result) {
        // Launch SSL connection
        std::make_shared<SecureHttpConnection>(std::move(socket_), ctx_,
                                               _web_server, std::move(buffer_))
            ->run();
      } else
        // Launch plain connection
        std::make_shared<PlainHttpConnection>(std::move(socket_), _web_server,
                                              std::move(buffer_))
            ->run();
    });
  }
};

}  // namespace dsa

#endif  // DSA_SDK_HTTP_CONNECTION_H

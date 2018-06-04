#ifndef DSA_SDK_WEBSOCKET_H
#define DSA_SDK_WEBSOCKET_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <mutex>

namespace websocket = boost::beast::websocket;
namespace http = boost::beast::http;

using tcp = boost::asio::ip::tcp;
namespace ssl = boost::asio::ssl;

typedef websocket::stream<tcp::socket> websocket_stream;
typedef websocket::stream<ssl::stream<tcp::socket&>> websocket_ssl_stream;

namespace dsa {

class Websocket : public std::enable_shared_from_this<Websocket> {
 public:
  typedef std::function<void(const boost::system::error_code& err,
                             size_t transferred)>
      Callback;
  typedef std::function<void(const boost::system::error_code& err)>
      ConnectCallback;

 private:
  bool _is_secure_stream;
  bool _is_websocket = false;
  tcp::socket _socket;
  std::unique_ptr<ssl::context> _ssl_context;
  std::unique_ptr<websocket_stream> _ws_stream;
  std::unique_ptr<websocket_ssl_stream> _wss_stream;
  bool _destroyed = false;
  std::mutex _mutex;

 public:
  Websocket(tcp::socket&& socket);
  Websocket(tcp::socket&& socket, ssl::context& ssl_context);
  ~Websocket();
  bool is_destroyed() const { return _destroyed; }
  bool is_secure_stream() const { return _is_secure_stream; }
  bool is_websocket() const { return _is_websocket; }

  void set_websocket() { _is_websocket = true; }
  tcp::socket& socket() { return _socket; }
  //  websocket_stream& stream() { return *_ws_stream; }
  //  websocket_ssl_stream& secure_stream() { return *_wss_stream; }

  void async_connect(const string_& hostname,
                     tcp::resolver::results_type& resolved,
                     ConnectCallback&& handler);

  void async_accept(
      boost::beast::http::request<boost::beast::http::string_body>& req,
      ConnectCallback&& handler);

  void async_read_some(boost::asio::mutable_buffer&& buffer,
                       Callback&& handler);
  void async_write(boost::asio::mutable_buffer&& buffer, Callback&& handler);

  void destroy();

  // httt
  void http_async_read(
      boost::beast::flat_buffer& buffer,
      boost::beast::http::request<boost::beast::http::string_body>& req,
      Callback&& handler);
  void http_async_write(http::response<http::string_body>& res,
                        Callback&& handler);
};
}  // namespace dsa

#endif  // DSA_SDK_WEBSOCKET_H

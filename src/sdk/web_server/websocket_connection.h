#ifndef DSA_SDK_WEBSOCKET_CONNECTION_H
#define DSA_SDK_WEBSOCKET_CONNECTION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>

#include <algorithm>
#include <boost/asio/bind_executor.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

#include <memory>

using tcp = boost::asio::ip::tcp;
namespace websocket = boost::beast::websocket;
namespace http = boost::beast::http;

namespace dsa {

class WebServer;
class Connection;

class WebsocketConnection
    : public std::enable_shared_from_this<WebsocketConnection> {
 private:
  WebServer& _web_server;
  std::shared_ptr<WebsocketConnection> _connection;
  boost::beast::websocket::stream<tcp::socket> _ws;
  boost::beast::flat_buffer _buffer;
  boost::beast::http::request<boost::beast::http::string_body> _req;
  boost::asio::steady_timer _timer;
  char ping_state_ = 0;

 public:
  explicit WebsocketConnection(WebServer& web_server, tcp::socket socket);
  template <class Body, class Allocator>
  void accept(boost::beast::http::request<
              Body, boost::beast::http::basic_fields<Allocator>>
                  req);

  void on_accept(boost::system::error_code ec);
  void on_timer(boost::system::error_code ec);
  void activity();
  void on_ping(boost::system::error_code ec);
  void do_read();
  void on_read(boost::system::error_code ec, std::size_t bytes_transferred);
  void on_write(boost::system::error_code ec, std::size_t bytes_transferred);
  // boost::asio::ip::tcp::socket& socket() { return _socket; }
  void destroy();
};

template <class Body, class Allocator>
void WebsocketConnection::accept(
    http::request<Body, http::basic_fields<Allocator>> req) {
  auto control_cb = [&](websocket::frame_type kind,
                        boost::beast::string_view payload) {
    boost::ignore_unused(kind, payload);
    activity();
  };
  _ws.control_callback(control_cb);

  on_timer({});

  _timer.expires_after(std::chrono::seconds(15));

  _ws.async_accept_ex(req,
                      [](websocket::response_type& res) {
                        res.insert(http::field::server, "dglux_server");
                      },
                      std::bind(&WebsocketConnection::on_accept,
                                shared_from_this(), std::placeholders::_1));
}

}  // namespace dsa

#endif  // DSA_SDK_WEBSOCKET_CONNECTION_H

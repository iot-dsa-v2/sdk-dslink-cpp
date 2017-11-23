#ifndef DSA_SDK_WEB_SERVER_H_
#define DSA_SDK_WEB_SERVER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include "network/ws/http_connection.h"

namespace dsa {

class App;

class WebServer : public std::enable_shared_from_this<WebServer> {
 private:
  uint16_t _port;
  string_ _doc_root;

  shared_ptr_<boost::asio::io_service> _io_service;
  shared_ptr_<boost::asio::io_service::strand> _strand;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
  shared_ptr_<HttpConnection> _next_connection;
  LinkStrandRef _link_strand;

  void accept_loop(const boost::system::error_code& error);

 public:
  typedef std::function<void(WebServer&)> HttpCallback;
  typedef std::function<void(WebServer&)> WsCallback;

  WebServer(App& app);
  ~WebServer();

  void start();
  void destroy();

  // HTTP server specific methods
  void add_http_handler(const string_& path, HttpCallback&& callback);
  void add_ws_handler(const string_& path, WsCallback&& callback);

  // util functions
 public:
  void send_error(int error_code, const string_ msg = "");
};

}  // namespace dsa

#endif  // DSA_SDK_WEB_SERVER_H_

#ifndef DSA_SDK_WEB_SERVER_H_
#define DSA_SDK_WEB_SERVER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

#include "http_connection.h"
#include "listener.h"

namespace dsa {

class App;

class WebServer : public std::enable_shared_from_this<WebServer> {
 private:
  boost::asio::io_service& _io_service;
  uint16_t _port;
  std::shared_ptr<Listener> _listener;

 public:
  typedef std::function<void(WebServer&)> HttpCallback;
  typedef std::function<void(WebServer&)> WsCallback;

  WebServer(App& app);
  ~WebServer();

  void listen(uint16_t port = 80);
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

#ifndef DSA_SDK_WEB_SERVER_H
#define DSA_SDK_WEB_SERVER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/http.hpp>

#include "listener.h"
#include "module/logger.h"
#include "websocket.h"

#include <map>

#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace dsa {

class App;
class Connection;

class WebServer : public std::enable_shared_from_this<WebServer> {
 public:
  typedef std::function<void(
      WebServer&, boost::asio::ip::tcp::socket&&,
      boost::beast::http::request<boost::beast::http::string_body>)>
      HttpCallback;
  typedef std::function<std::shared_ptr<Connection>(
      WebServer&, Websocket,
      boost::beast::http::request<boost::beast::http::string_body>)>
      WsCallback;

 private:
  boost::asio::io_service& _io_service;
  uint16_t _port;
  std::shared_ptr<Listener> _listener;
  uint16_t _secure_port;
  std::shared_ptr<Listener> _secure_listener;

  // http/ws callbacks
  typedef std::pair<const string_, WsCallback&&> WsCallbackPair;
  typedef std::map<const string_, WsCallback&&> WsCallbackMap;
  WsCallbackMap _ws_callback_map;

 public:
  WebServer(App& app);
  ~WebServer();

  void listen(uint16_t port = 80);
  void secure_listen(uint16_t port = 443);
  void start();
  void destroy();
  boost::asio::io_service& io_service() { return _io_service; }

  // HTTP server specific methods
  void add_http_handler(const string_& path, HttpCallback&& callback);
  void add_ws_handler(const string_& path, WsCallback&& callback);
  HttpCallback& http_handler(const string_& path);
  WsCallback& ws_handler(const string_& path);

  // util functions
  void send_error(int error_code, const string_ msg = "");
};

class ErrorCallback {
 private:
  uint16_t _error_code;

 public:
  ErrorCallback(uint16_t error_code) : _error_code(error_code) {}

  void operator()(
      WebServer& web_server, Websocket,
      boost::beast::http::request<boost::beast::http::string_body>&& req) {
    // TODO - construct a proper http response
    LOG_ERROR(Logger::_(), LOG << "http error code: " << _error_code);
  }
};

}  // namespace dsa

#endif  // DSA_SDK_WEB_SERVER_H

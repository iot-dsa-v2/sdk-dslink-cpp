#ifndef DSA_SDK_WEB_SERVER_H
#define DSA_SDK_WEB_SERVER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/http.hpp>
#include <map>

#include "listener.h"
#include "login_manager.h"
#include "module/logger.h"
#include "websocket.h"
#include "http_request.h"

#include <map>

#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>

namespace dsa {

class App;
class Connection;
class HttpRequest;

class WebServer : public std::enable_shared_from_this<WebServer> {
 public:
  typedef std::function<void(
      WebServer&, HttpRequest&&)>
      HttpCallback;
  typedef std::function<std::shared_ptr<Connection>(
      WebServer&, std::unique_ptr<Websocket>&&,
      http::request<request_body_t, http::basic_fields<alloc_t>>&&)>
      WsCallback;
 private:
  boost::asio::io_service& _io_service;
  uint16_t _port;
  std::shared_ptr<Listener> _listener;
  shared_ptr_<LoginManager> _login_mngr;
  uint16_t _secure_port;
  std::shared_ptr<Listener> _secure_listener;
  boost::asio::ssl::context _ssl_context;

  // http/ws callbacks
  typedef std::pair<const string_, WsCallback&&> WsCallbackPair;
  typedef std::map<const string_, WsCallback&&> WsCallbackMap;
  WsCallbackMap _ws_callback_map;

  typedef std::pair<const string_, HttpCallback&&> HttpCallbackPair;
  typedef std::map<const string_, HttpCallback&&> HttpCallbackMap;
  HttpCallbackMap _http_callback_map;

 public:
  WebServer(App& app, shared_ptr_<LoginManager> login_mngr = nullptr);
  ~WebServer();

  void listen(uint16_t port = 80);
  void secure_listen(uint16_t port = 443);
  void start();
  void destroy();
  boost::asio::io_service& io_service() { return _io_service; }
  boost::asio::ssl::context& ssl_context() { return _ssl_context; }

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
  explicit ErrorCallback(uint16_t error_code) : _error_code(error_code) {}

  void operator()(
      WebServer& web_server, Websocket,
      HttpRequest&& req) {
    // TODO - construct a proper http response
    LOG_ERROR(__FILENAME__, LOG << "http error code: " << _error_code);
  }
};

}  // namespace dsa

#endif  // DSA_SDK_WEB_SERVER_H

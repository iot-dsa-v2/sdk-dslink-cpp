#ifndef DSA_SDK_WEB_SERVER_H
#define DSA_SDK_WEB_SERVER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include "core/editable_strand.h"
#include "listener.h"
#include "module/logger.h"
#include "websocket.h"

namespace dsa {

class App;
class Connection;
class HttpConnection;

class WebServer : public std::enable_shared_from_this<WebServer> {
  friend class HttpConnection;
 public:
  typedef std::function<void(const string_& dsid, const string_& token,
                             const string_& body)>
      V1ConnCallback;
  typedef std::function<void(std::unique_ptr<Websocket>&&)> V1WsCallback;

 private:
  boost::asio::io_service& _io_service;
  SharedLinkStrandRef _shared_strand;
  uint16_t _port;
  std::shared_ptr<Listener> _listener;
  int32_t _secure_port;
  std::shared_ptr<Listener> _secure_listener;
  boost::asio::ssl::context _ssl_context;

  V1ConnCallback _v1_conn_callback;
  V1WsCallback _v1_ws_callback;

 public:
  WebServer(App& app, const LinkStrandRef& strand);
  ~WebServer();

  void initV1Connection(V1ConnCallback&& conn_callback,
                        V1WsCallback&& ws_callback);

  void listen(uint16_t port = 80);
  void secure_listen(uint16_t port = 443);
  void start();
  void destroy();
  boost::asio::io_service& io_service() { return _io_service; }
  boost::asio::ssl::context& ssl_context() { return _ssl_context; }
  SharedLinkStrandRef get_shared_strand() { return _shared_strand; }

  // util functions
  void send_error(int error_code, const string_ msg = "");
};

}  // namespace dsa

#endif  // DSA_SDK_WEB_SERVER_H

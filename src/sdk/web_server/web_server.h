#ifndef DSA_SDK_WEB_SERVER_H_
#define DSA_SDK_WEB_SERVER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>
#include <functional>
#include <memory>
#include "core/server.h"

using tcp = boost::asio::ip::tcp;

namespace dsa {

class App;

class HttpSession : public std::enable_shared_from_this<HttpSession> {
 private:
  tcp::socket _socket;
  boost::asio::io_service::strand _strand;
  std::string _doc_root;

 public:
  HttpSession(tcp::socket socket, std::string const& doc_root)
      : _socket(std::move(socket)),
        _strand(_socket.get_io_service()),
        _doc_root(doc_root) {}

  ~HttpSession() = default;
  void start();
};

class Listener : public std::enable_shared_from_this<Listener> {
 private:
  tcp::acceptor _acceptor;
  tcp::socket _socket;
  std::string const& _doc_root;

 public:
  Listener(boost::asio::io_service& ios, tcp::endpoint endpoint,
           std::string const& doc_root);
  ~Listener();

  void start();
};

class WebServer : public std::enable_shared_from_this<WebServer> {
 private:
  App& _app;

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

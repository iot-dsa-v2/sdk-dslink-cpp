#ifndef DSA_SDK_LISTENER_H_
#define DSA_SDK_LISTENER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/io_service.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <memory>

#include "http_connection.h"

namespace dsa {

class WebServer;

class Listener : public std::enable_shared_from_this<Listener> {
 private:
  WebServer& _web_server;
  uint16_t _port;
  boost::asio::io_service& _io_service;
  std::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
  std::shared_ptr<HttpConnection> _next_connection;

 public:
  Listener(WebServer& web_server, boost::asio::io_service& io_service,
           uint16_t port);
  ~Listener();

  void run();
  void accept_loop(const boost::system::error_code& error);
  void destroy();
};
}  // namespace dsa

#endif  // DSA_SDK_LISTENER_H_

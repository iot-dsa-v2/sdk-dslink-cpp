#ifndef DSA_SDK_LISTENER_H
#define DSA_SDK_LISTENER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>

#include <memory>
#include <mutex>

namespace dsa {

class WebServer;

class Listener : public std::enable_shared_from_this<Listener> {
 private:
  WebServer& _web_server;
  uint16_t _port;
  bool _is_secured;
  std::shared_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
  std::mutex _mutex;
  bool _destroyed = false;
  boost::asio::ip::tcp::socket _socket;

 public:
  Listener(WebServer& web_server, uint16_t port, bool is_secured = true);
  ~Listener();

  void run();
  void accept_loop(const boost::system::error_code& error);
  void destroy();
};
}  // namespace dsa

#endif  // DSA_SDK_LISTENER_H

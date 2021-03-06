#ifndef DSA_SDK_TCP_SERVER_H
#define DSA_SDK_TCP_SERVER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>

#include "core/server.h"
#include "core/session.h"
#include "util/app.h"
#include "util/enable_shared.h"

namespace dsa {
class TcpServerConnection;
class StcpServerConnection;

class TcpServer : public Server {
 private:
  int32_t _port;
  int32_t _secure_port;

  std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
  shared_ptr_<TcpServerConnection> _next_connection;

  std::unique_ptr<boost::asio::ip::tcp::acceptor> _secure_acceptor;
  shared_ptr_<StcpServerConnection> _secure_next_connection;

  void accept_loop(const boost::system::error_code &error);
  void secure_accept_loop(const boost::system::error_code &error);

 public:
  TcpServer(WrapperStrand &config);
  ~TcpServer();

  void start() override;
  void destroy_impl() override;
  string_ type() override { return "TCP"; }

  int get_port();

  int get_secure_port();
};

typedef shared_ptr_<TcpServer> TcpServerPtr;
}  // namespace dsa

#endif  // DSA_SDK_TCP_SERVER_H

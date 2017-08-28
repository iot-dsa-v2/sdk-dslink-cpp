#ifndef DSA_SDK_TCP_SERVER_H_
#define DSA_SDK_TCP_SERVER_H_

#include <boost/asio/ip/tcp.hpp>


#include "core/session.h"
#include "core/server.h"
#include "core/app.h"
#include "util/enable_shared.h"

namespace dsa {
class TcpServerConnection;

class TcpServer : public Server {
 private:
  std::string _hostname;
  uint16_t _port;
  uint32_t _handshake_timeout_ms = 5000;

  std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
  shared_ptr_<TcpServerConnection> _next_connection;

  void accept_loop(const boost::system::error_code &error);

 public:
  TcpServer(WrapperConfig &config);
  void start() override;
  void close_impl() override;
  std::string type() override { return "TCP"; }
};

typedef shared_ptr_<TcpServer> TcpServerPtr;
}  // namespace dsa

#endif  // DSA_SDK_TCP_SERVER_H_

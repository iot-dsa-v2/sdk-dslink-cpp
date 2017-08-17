#ifndef DSA_SDK_TCP_SERVER_H_
#define DSA_SDK_TCP_SERVER_H_


#include <boost/asio.hpp>

#include "core/session.h"
#include "core/server.h"
#include "core/app.h"
#include "util/enable_shared.h"

namespace dsa {
class TcpServerConnection;

class TcpServer : public Server {
 private:
  std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
  shared_ptr_<TcpServerConnection> _new_connection;

  void accept_loop(const boost::system::error_code &error);

 public:
  TcpServer(const Config &config);
  void start() override;
  void close() override;
  std::string type() override { return "TCP"; }
};

typedef shared_ptr_<TcpServer> TcpServerPtr;
}  // namespace dsa

#endif  // DSA_SDK_TCP_SERVER_H_

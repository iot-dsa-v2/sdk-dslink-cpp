#ifndef DSA_SDK_TCP_SERVER_H_
#define DSA_SDK_TCP_SERVER_H_


#include <boost/asio.hpp>

#include "session.h"
#include "server.h"
#include "app.h"
#include "util/enable_shared.h"

namespace dsa {
class TcpServerConnection;

class TcpServer : public Server {
 private:
  std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
  shared_ptr_<TcpServerConnection> _new_connection;

  void accept_loop(const boost::system::error_code &error);

  Config _config;

 public:
  TcpServer(shared_ptr_<App> app, const Config &config);
  void start() override;
  void close() override;
  std::string type() override { return "TCP"; }
};

typedef shared_ptr_<TcpServer> TcpServerPtr;
}  // namespace dsa

#endif  // DSA_SDK_TCP_SERVER_H_

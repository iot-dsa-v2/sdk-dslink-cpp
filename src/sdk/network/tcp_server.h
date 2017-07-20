#ifndef DSA_SDK_TCP_SERVER_H_
#define DSA_SDK_TCP_SERVER_H_

#include <memory>

#include <boost/asio.hpp>

#include "server.h"
#include "app.h"
#include "util/enable_shared.h"

namespace dsa {
class TcpServerConnection;

class TcpServer : public Server {
 private:
  friend class TcpServerConnection;
  std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;

  void accept_loop();

  Config _config;

 public:
  TcpServer(const App &app, const Config &config);
  void start() override;
  std::string type() override { return "TCP"; }
};

typedef std::shared_ptr<TcpServer> TcpServerPtr;
}  // namespace dsa

#endif  // DSA_SDK_TCP_SERVER_H_

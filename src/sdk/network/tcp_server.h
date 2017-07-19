#ifndef DSA_SDK_TCP_SERVER_H_
#define DSA_SDK_TCP_SERVER_H_

#include <memory>

#include <boost/asio.hpp>

#include "server.h"
#include "app.h"

namespace dsa {
class TcpServerConnection;

class TcpServer : public Server {
 private:
  friend class TcpServerConnection;
  boost::asio::ip::tcp::acceptor _acceptor;

  void accept_loop();

 public:
  TcpServer(const App &app, unsigned short port);

  void start() override;
};

typedef std::shared_ptr<TcpServer> TcpServerPtr;
}  // namespace dsa

#endif  // DSA_SDK_TCP_SERVER_H_

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
  class Config {
   private:
    unsigned short _port{8080};

   public:
    void set_port(unsigned short port) { _port = port; }
    unsigned short port() const { return _port; }
  };

  TcpServer(const App &app, const Config &config);
  void start() override;

  const Config config;
};

typedef std::shared_ptr<TcpServer> TcpServerPtr;
}  // namespace dsa

#endif  // DSA_SDK_TCP_SERVER_H_

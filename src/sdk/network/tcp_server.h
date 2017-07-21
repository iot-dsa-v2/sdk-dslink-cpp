#ifndef DSA_SDK_TCP_SERVER_H_
#define DSA_SDK_TCP_SERVER_H_

#include <memory>

#include <boost/asio.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "session.h"
#include "server.h"
#include "app.h"
#include "util/enable_shared.h"

namespace dsa {
class TcpServerConnection;

class TcpServer : public Server {
 private:
  std::unique_ptr<boost::asio::ip::tcp::acceptor> _acceptor;
  boost::shared_mutex _sessions_key;
  std::shared_ptr<TcpServerConnection> _new_connection;

  void accept_loop(const boost::system::error_code &error);

  Config _config;

 public:
  TcpServer(const App &app, const Config &config);
  ~TcpServer() override;
  void start() override;
  std::string type() override { return "TCP"; }

  SessionPtr get_session(const std::string &session_id) override;
  SessionPtr create_session() override;
  std::string get_new_session_id() override;
};

typedef std::shared_ptr<TcpServer> TcpServerPtr;
}  // namespace dsa

#endif  // DSA_SDK_TCP_SERVER_H_

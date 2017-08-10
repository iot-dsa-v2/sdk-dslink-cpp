//
// Created by Ben Richards on 8/9/17.
//

#ifndef DSA_SDK_NETWORK_CONNECTION_SERVER_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_SERVER_CONNECTION_H_

#include <boost/asio/strand.hpp>

#include "core/connection.h"

namespace dsa {
class Server;

class ServerConnection : virtual public Connection {
 public:
  ServerConnection(boost::asio::io_service::strand &strand, const Config &config);

 protected:
  void on_connect() throw(const std::runtime_error &) override;

  // handshake functions
  bool parse_f0(size_t size);
  bool parse_f2(size_t size);
  size_t load_f1(Buffer &buf);
  size_t load_f3(Buffer &buf);

  // weak pointer needed here in order for the server to be able to be freed
  // once TcpServer::stop is called. std::weak_ptr::lock implementation just
  // copies a shared pointer so performance cost should be minimal. this pointer
  // should rarely be touched by connection.
  std::weak_ptr<Server> _server;
};
}  // namespace dsa

#endif // DSA_SDK_NETWORK_CONNECTION_SERVER_CONNECTION_H_

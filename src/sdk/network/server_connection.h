#ifndef DSA_SDK_NETWORK_CONNECTION_SERVER_CONNECTION_H_
#define DSA_SDK_NETWORK_CONNECTION_SERVER_CONNECTION_H_


#include "connection.h"

namespace dsa {
class Server;

class ServerConnection : virtual public Connection {
 public:
  ServerConnection(LinkStrandPtr strand, uint32_t handshake_timeout_ms,
                   const std::string &dsid_prefix,
                   const std::string &path = "");

 protected:
  void on_connect() throw(const std::runtime_error &) override;

  // handshake functions
  bool parse_f0(size_t size);
  bool parse_f2(size_t size);
  size_t load_f1(ByteBuffer &buf);
  size_t load_f3(ByteBuffer &buf);

  // weak pointer needed here in order for the server to be able to be freed
  // once TcpServer::stop is called. std::weak_ptr::lock implementation just
  // copies a shared pointer so performance cost should be minimal. this pointer
  // should rarely be touched by connection.
  shared_ptr_<Server> _server;
};
}  // namespace dsa

#endif // DSA_SDK_NETWORK_CONNECTION_SERVER_CONNECTION_H_

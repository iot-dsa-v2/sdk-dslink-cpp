#ifndef DSA_SDK_NETWORK_CONNECTION_CLIENT_CONNECTION_H
#define DSA_SDK_NETWORK_CONNECTION_CLIENT_CONNECTION_H

#include "core/connection.h"

#include "util/enable_intrusive.h"

namespace dsa {
class ClientConnection : virtual public Connection {
 public:
  ClientConnection(LinkStrandPtr strand, uint32_t handshake_timeout_ms,
                   const std::string &dsid_prefix, const std::string &path = "",
                   const std::string &client_token = "");

  virtual ~ClientConnection() = default;

 protected:
  std::string _client_token;

  void on_connect() throw(const std::runtime_error &) override;

  // handshake functions
  bool parse_f1(size_t size);
  bool parse_f3(size_t size);
  size_t load_f0(ByteBuffer &buf);
  size_t load_f2(ByteBuffer &buf);
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_CLIENT_CONNECTION_H

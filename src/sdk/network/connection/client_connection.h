#ifndef DSA_SDK_NETWORK_CONNECTION_CLIENT_CONNECTION_H
#define DSA_SDK_NETWORK_CONNECTION_CLIENT_CONNECTION_H

#include <boost/asio/strand.hpp>

#include "core/connection.h"

namespace dsa {
class ClientConnection : virtual public Connection {
 public:
  ClientConnection(boost::asio::io_service::strand &strand, const Config &config);

 protected:
  void on_connect() throw(const std::runtime_error &) override;

  // handshake functions
  bool parse_f1(size_t size);
  bool parse_f3(size_t size);
  size_t load_f0(Buffer &buf);
  size_t load_f2(Buffer &buf);
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_CONNECTION_CLIENT_CONNECTION_H

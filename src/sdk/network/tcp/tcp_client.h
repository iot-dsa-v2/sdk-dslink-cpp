#ifndef DSA_SDK_TCP_CLIENT_H
#define DSA_SDK_TCP_CLIENT_H

#include <boost/asio.hpp>

#include "core/client.h"

namespace dsa {
class TcpClient : public Client {
 protected:
  std::string _hostname;
  uint16_t _port;

 public:
  TcpClient(const Config &config);

  const std::string &get_hostname() const { return _hostname; }
  uint16_t get_port() const { return _port; }

  void connect() override;
};
}  // namespace dsa

#endif  // !DSA_SDK_TCP_CLIENT_H

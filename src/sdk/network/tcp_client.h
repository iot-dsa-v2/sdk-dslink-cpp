#ifndef DSA_SDK_TCP_CLIENT_H
#define DSA_SDK_TCP_CLIENT_H

#include <boost/asio.hpp>

#include "core/client.h"

namespace dsa {
class TcpClient : public Client {
 public:
  TcpClient(boost::asio::io_service::strand &strand, const Config &config);
  void connect() override;
};
}  // namespace dsa

#endif  // !DSA_SDK_TCP_CLIENT_H

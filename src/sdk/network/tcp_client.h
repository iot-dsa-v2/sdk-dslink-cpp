#ifndef DSA_SDK_TCP_CLIENT_H
#define DSA_SDK_TCP_CLIENT_H

#include "core/client.h"

namespace dsa {
class TcpClient : public Client {
 public:
  TcpClient(const App &app, const Config &config);
  void connect() override;
};
}  // namespace dsa

#endif  // !DSA_SDK_TCP_CLIENT_H

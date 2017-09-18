#ifndef DSA_SDK_TCP_CLIENT_H
#define DSA_SDK_TCP_CLIENT_H

#if defined(_MSC_VER)
#pragma once
#endif



#include "core/client.h"

namespace dsa {
class TcpClient : public Client {
 protected:
  std::string _hostname;
  uint16_t _port;
  uint32_t _handshake_timeout_ms = 5000;

 public:
  TcpClient(WrapperConfig &config);

  const std::string &get_hostname() const { return _hostname; }
  uint16_t get_port() const { return _port; }

  void connect() override;
};
}  // namespace dsa

#endif  // !DSA_SDK_TCP_CLIENT_H

#ifndef DSA_SDK_TCP_CONNECTION_H_
#define DSA_SDK_TCP_CONNECTION_H_

#include "enabled_shared.h"

namespace dsa {
class Connection {
  virtual void destory();
};

class TcpConnection : public Connection {};

class TcpClientConnection : public TcpConnection,
                            public EnableShared<TcpClientConnection> {};

class TcpServerConnection : public TcpConnection,
                            public EnableShared<TcpServerConnection> {};
}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H_

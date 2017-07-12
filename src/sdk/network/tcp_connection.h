#ifndef DSA_SDK_TCP_CONNECTION_H_
#define DSA_SDK_TCP_CONNECTION_H_

namespace dsa {
class Connection;

class TcpConnection : public Connection {};

class TcpClientConnection : public TcpConnection {};

class TcpServerConnection : public TcpConnection {};
}  // namespace dsa

#endif  // DSA_SDK_TCP_CONNECTION_H_

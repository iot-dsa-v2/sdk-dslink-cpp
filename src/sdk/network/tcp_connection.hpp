#pragma once

#include "connection.hpp"

namespace dsa {

class TcpConnection : public Connection {

};

class TcpClientConnection : public TcpConnection {

};

class TcpServerConnection : public TcpConnection {

};

}

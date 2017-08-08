#include "dsa_common.h"

#include "tcp_client.h"

namespace dsa {
TcpClient::TcpClient(const App &app, const Config &config)
    : Client(app, config){};

void TcpClient::connect() {
	
}
}  // namespace dsa

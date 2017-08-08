#include "dsa_common.h"

#include "tcp_client.h"
#include "tcp_connection.h"
#include "core/session.h"

namespace dsa {
TcpClient::TcpClient(boost::asio::io_service::strand &strand, const Config &config)
    : Client(strand, config) {
  _connection = make_shared_<TcpClientConnection>(strand, config, [=](intrusive_ptr_<Session> session) {
  	_session = std::move(session);
  	_session->start();
  });
};

void TcpClient::connect() {
  _connection->connect();
}
}  // namespace dsa

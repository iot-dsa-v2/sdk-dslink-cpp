#include "dsa_common.h"

#include "client.h"

namespace dsa {
Client::Client(boost::asio::io_service::strand &strand, const Config &config)
    : _strand(strand), _config(config), _session(make_intrusive_<Session>(strand, "", config)) {}
void Client::close() {
  _connection->close();
}
}  // namespace dsa

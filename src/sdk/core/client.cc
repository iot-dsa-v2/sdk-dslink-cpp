#include "dsa_common.h"

#include "client.h"

namespace dsa {
Client::Client(boost::asio::io_service::strand &strand, const Config &config)
    : _strand(strand), _config(config), _session(make_intrusive_<Session>(strand, make_intrusive_<Buffer>(""))) {}
void Client::close() {
  _connection->close();
}
}  // namespace dsa

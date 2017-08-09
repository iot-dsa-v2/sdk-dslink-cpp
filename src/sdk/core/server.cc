#include "dsa_common.h"

#include "app.h"
#include "server.h"
#include "session.h"
#include "session_manager.h"

namespace dsa {

Server::Server(boost::asio::io_service::strand &strand, const Config &config)
    : _strand(strand),
      _config(config),
      _session_manager(strand) {}

void Server::on_session_connected(const shared_ptr_<Session> &session) {}

void Server::close() {
  _session_manager.end_all_sessions();	
}

}  // namespace dsa
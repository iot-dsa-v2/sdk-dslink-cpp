#include "dsa_common.h"

#include "ws_server_connection.h"

#include <boost/bind.hpp>

// TODO: remove this
#include <boost/asio.hpp>

// TODO
//#include "tcp_server.h"

#include "message/handshake/f1_message.h"

#define DEBUG 0

namespace dsa {
WsServerConnection::WsServerConnection(LinkStrandRef &strand,
                                         const string_ &dsid_prefix,
                                         const string_ &path)
    : WsConnection(strand, dsid_prefix, path) {}

void WsServerConnection::accept() {
  {
    std::lock_guard<std::mutex> lock(mutex);
    on_read_message = [this](MessageRef message) {
      return on_receive_f0(std::move(message));
    };
  }
  WsConnection::start_read(share_this<WsServerConnection>(), 0, 0);
  start_deadline_timer(15);
}

}  // namespace dsa

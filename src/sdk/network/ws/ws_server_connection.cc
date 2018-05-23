#include "dsa_common.h"

#include "ws_server_connection.h"

#define DEBUG 0

namespace dsa {
WsServerConnection::WsServerConnection(shared_ptr_<Websocket> websocket,
                                       const SharedLinkStrandRef &strand,
                                       const string_ &dsid_prefix,
                                       const string_ &path)
    : WsConnection(strand, dsid_prefix, path) {
  _websocket = std::move(websocket);
}

void WsServerConnection::accept() {
  {
    std::lock_guard<std::mutex> lock(mutex);
    on_read_message = [this](MessageRef message) {
      on_receive_f0(std::move(message));
    };
  }

  WsConnection::start_read(share_this<WsServerConnection>());
  start_deadline_timer(15);
}

}  // namespace dsa

#include "dsa_common.h"

#include "wss_server_connection.h"

#define DEBUG 0

namespace dsa {
WssServerConnection::WssServerConnection(websocket_ssl_stream &stream,
                                         LinkStrandRef &strand,
                                         const string_ &dsid_prefix,
                                         const string_ &path)
    : WssConnection(stream, strand, dsid_prefix, path) {}

void WssServerConnection::accept() {
  {
    std::lock_guard<std::mutex> lock(mutex);
    on_read_message = [this](MessageRef message) {
      on_receive_f0(std::move(message));
    };
  }

  WssConnection::start_read(share_this<WssServerConnection>());
  start_deadline_timer(15);
}

}  // namespace dsa

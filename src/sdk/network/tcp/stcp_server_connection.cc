#include "dsa_common.h"

#include "module/logger.h"
#include "stcp_server_connection.h"

#define DEBUG 0

namespace dsa {

StcpServerConnection::StcpServerConnection(LinkStrandRef &strand,
                                           boost::asio::ssl::context &context,
                                           const string_ &dsid_prefix,
                                           const string_ &path)
    : StcpConnection(strand, context, dsid_prefix, path) {}

void StcpServerConnection::accept() {
  boost::system::error_code ec;
  _socket.handshake(boost::asio::ssl::stream_base::server, ec);
  if (ec) {
    LOG_FATAL(LOG << "Failed to establish SSL handshake");
  }

  {
    std::lock_guard<std::mutex> lock(mutex);
    on_read_message = [this](MessageRef message) {
      on_receive_f0(std::move(message));
    };
  }

  StcpConnection::start_read(share_this<StcpServerConnection>(), 0, 0);
  start_deadline_timer(15);
}

}  // namespace dsa

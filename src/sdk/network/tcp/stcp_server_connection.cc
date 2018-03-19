#include "dsa_common.h"

#include "module/logger.h"
#include "stcp_server_connection.h"

#define DEBUG 0

namespace dsa {

StcpServerConnection::StcpServerConnection(const LinkStrandRef &strand,
                                           boost::asio::ssl::context &context,
                                           const string_ &dsid_prefix,
                                           const string_ &path)
    : StcpConnection(strand, context, dsid_prefix, path) {}

void StcpServerConnection::accept() {
  _socket.async_handshake(boost::asio::ssl::stream_base::server, [
    this, sthis = shared_from_this()
  ](const boost::system::error_code &error)->void {
    return handle_handshake(error);
  });
}

void StcpServerConnection::handle_handshake(
    const boost::system::error_code &error) {
  if (error != boost::system::errc::success) {
    LOG_ERROR(__FILENAME__,
              LOG << "Server SSL handshake failed: " << error << "\n");
  } else {
    {
      std::lock_guard<std::mutex> lock(mutex);
      on_read_message = [this](MessageRef message) {
        on_receive_f0(std::move(message));
      };
    }

    StcpConnection::start_read(share_this<StcpServerConnection>());
    start_deadline_timer(15);
  }
}

}  // namespace dsa

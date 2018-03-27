#include "dsa_common.h"

#include "tcp_server_connection.h"

#include <boost/bind.hpp>

// TODO: remove this
#include <boost/asio.hpp>

#include "tcp_server.h"

#include "message/handshake/f1_message.h"

#define DEBUG 0

namespace dsa {
TcpServerConnection::TcpServerConnection(const SharedLinkStrandRef &strand,
                                         const string_ &dsid_prefix,
                                         const string_ &path)
    : TcpConnection(strand, dsid_prefix, path) {}

void TcpServerConnection::accept() {
  {
    std::lock_guard<std::mutex> lock(mutex);
    on_read_message = [this](MessageRef message) {
      on_receive_f0(std::move(message));
    };
  }
  TcpConnection::start_read(share_this<TcpServerConnection>());
  start_deadline_timer(15);
}

}  // namespace dsa

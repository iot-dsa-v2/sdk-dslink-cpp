#include "dsa_common.h"

#include "tcp_server_connection.h"

#include <boost/bind.hpp>

// TODO: remove this
#include <boost/asio.hpp>

#include "tcp_server.h"

#include "message/handshake/f1_message.h"

#define DEBUG 0

namespace dsa {
TcpServerConnection::TcpServerConnection(LinkStrandRef &strand,
                                         uint32_t handshake_timeout_ms,
                                         const string_ &dsid_prefix,
                                         const string_ &path)
    : TcpConnection(strand, handshake_timeout_ms, dsid_prefix, path) {}

void TcpServerConnection::accept() {
#if 0
  // TODO fix this
  // start timeout timer with handshake timeout specified in config
  _deadline.expires_from_now(
      boost::posix_time::milliseconds(_handshake_timeout_ms));
  _deadline.async_wait(boost::bind(&TcpServerConnection::timeout,
                                   share_this<TcpServerConnection>(),
                                   boost::asio::placeholders::error));
#endif
  {
    boost::unique_lock<boost::shared_mutex>(read_loop_mutex);
    on_read_message = [this](MessageRef message) {
      return on_receive_f0(std::move(message));
    };
  }
  TcpConnection::start_read(share_this<TcpServerConnection>(), 0, 0);
}

}  // namespace dsa

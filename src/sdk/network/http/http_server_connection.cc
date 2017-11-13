#include "dsa_common.h"

#include "http_server_connection.h"

#include <boost/bind.hpp>

// TODO: remove this
#include <boost/asio.hpp>

// TODO
//#include "tcp_server.h"

#include "message/handshake/f1_message.h"

#define DEBUG 0

namespace dsa {
HttpServerConnection::HttpServerConnection(LinkStrandRef &strand,
                                         const string_ &dsid_prefix,
                                         const string_ &path)
    : HttpConnection(strand, dsid_prefix, path) {}

void HttpServerConnection::accept() {
  {
    std::lock_guard<std::mutex> lock(mutex);
    on_read_message = [this](MessageRef message) {
      return on_receive_f0(std::move(message));
    };
  }
  HttpConnection::start_read(share_this<HttpServerConnection>(), 0, 0);
  start_deadline_timer(15);
}

}  // namespace dsa

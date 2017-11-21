#include "dsa_common.h"

#include "ws_server_connection.h"

#include <boost/beast/http.hpp>
#include <boost/beast/http/message.hpp>

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

  // Read a request
  http::async_read(
      _socket, _buffer, _req,
      // TODO: run within the strand?
      [ connection = share_this<WsServerConnection>(), this ](
          const boost::system::error_code &error, size_t bytes_transferred) {

        // TODO: check error/termination conditions

        if (websocket::is_upgrade(_req)) {
          // accept the websocket handshake
          _ws.async_accept(_req, [this](
                                     const boost::system::error_code &error) {

            // TODO: run within the strand?

            WsConnection::start_read(share_this<WsServerConnection>(), 0, 0);

            return;
          });  // async_accept
        }
        return;
      });  // async_read

  start_deadline_timer(15);
}

}  // namespace dsa

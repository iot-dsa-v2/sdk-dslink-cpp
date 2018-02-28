#include "dsa_common.h"

#include "ws_client_connection.h"

#include <boost/asio/connect.hpp>

#include "module/logger.h"

namespace dsa {
WsClientConnection::WsClientConnection(LinkStrandRef &strand,
                                       const string_ &dsid_prefix,
                                       const string_ &host, uint16_t port)
    : WsConnection(*make_shared_<websocket_stream>(strand->get_io_context()),
                   strand, dsid_prefix),
      _hostname(host),
      _port(port) {}

void WsClientConnection::connect(size_t reconnect_interval) {
  // connect to server
  using tcp = boost::asio::ip::tcp;
  tcp::resolver resolver(_strand->get_io_context());
  // TODO: timeout
  LOG_FINE(__FILENAME__,
           LOG << "WS client connecting to " << _hostname << ":" << _port);

  tcp::resolver::results_type results =
      resolver.resolve(tcp::resolver::query(_hostname, std::to_string(_port)));
  boost::asio::async_connect(
      _socket.next_layer(), results.begin(), results.end(),
      // capture shared_ptr to keep the instance
      // capture this to access protected member
      [ connection = share_this<WsConnection>(), this ](
          const boost::system::error_code &error,
          tcp::resolver::iterator) mutable {
        if (is_destroyed()) return;
        if (error != boost::system::errc::success) {
          destroy_in_strand(std::move(connection));
          // TODO: log or return the error?
          return;
        }

        // websocket handshake
        _socket.async_handshake(_hostname, "/", [
          connection = connection, this
        ](const boost::system::error_code &error) mutable {
          if (is_destroyed()) return;
          if (error != boost::system::errc::success) {
            destroy_in_strand(std::move(connection));
            LOG_ERROR(__FILENAME__, LOG << "Client websocket handshake failed");
            return;
          }

          start_client_f0();

          WsConnection::start_read(std::move(connection));

        });
      });

  // use half of the reconnection time to resolve host
  start_deadline_timer((reconnect_interval >> 1) + 1);
}

}  // namespace dsa

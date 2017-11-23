#include "dsa_common.h"

#include "ws_client_connection.h"

#include <boost/asio/connect.hpp>

#include "module/logger.h"

namespace dsa {
WsClientConnection::WsClientConnection(LinkStrandRef &strand,
                                       const string_ &dsid_prefix,
                                       const string_ &host, uint16_t port)
    : WsConnection(*new websocket_stream{strand->get_io_service()}, strand,
                   dsid_prefix),
      _hostname(host),
      _port(port) {}

void WsClientConnection::connect(size_t reconnect_interval) {
  // connect to server
  using tcp = boost::asio::ip::tcp;
  tcp::resolver resolver(_strand->get_io_service());
  // TODO: timeout
  LOG_INFO(_strand->logger(),
           LOG << "TCP client connecting to " << _hostname << ":" << _port);
  boost::asio::async_connect(
      _ws.next_layer(),
      resolver.resolve(tcp::resolver::query(_hostname, std::to_string(_port))),
      // capture shared_ptr to keep the instance
      // capture this to access protected member
      [ connection = share_this<WsConnection>(), this ](
          const boost::system::error_code &error,
          tcp::resolver::iterator) mutable {
        if (is_destroyed()) return;
        /* TODO
        if (error != boost::system::errc::success) {
          TcpConnection::destroy_in_strand(std::move(connection));
          // TODO: log or return the error?
          return;
        }
        */

        // websocket handshake
        _ws.async_handshake(_hostname, "/", [
          connection = share_this<WsConnection>(), this
        ](const boost::system::error_code &error) mutable {
          if (is_destroyed()) return;

          start_client_f0();

          WsConnection::start_read(std::move(connection));

        });

      });
  start_deadline_timer(reconnect_interval);
}

}  // namespace dsa

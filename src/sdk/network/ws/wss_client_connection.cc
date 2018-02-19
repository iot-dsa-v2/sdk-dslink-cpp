#include "dsa_common.h"

#include "wss_client_connection.h"

#include <boost/asio/connect.hpp>

#include "module/logger.h"

namespace dsa {
WssClientConnection::WssClientConnection(websocket_ssl_stream &stream,
                                         LinkStrandRef &strand,
                                         const string_ &dsid_prefix,
                                         const string_ &host, uint16_t port)
    : WssConnection(stream, strand, dsid_prefix),
      _hostname(host),
      _port(port) {}

void WssClientConnection::connect(size_t reconnect_interval) {
  // connect to server
  using tcp = boost::asio::ip::tcp;
  tcp::resolver resolver(_strand->get_io_context());
  // TODO: timeout
  LOG_DEBUG(
      Logger::_(),
      LOG << "WS secure client connecting to " << _hostname << ":" << _port);

  tcp::resolver::results_type results =
      resolver.resolve(tcp::resolver::query(_hostname, std::to_string(_port)));

  boost::asio::async_connect(
      _socket.next_layer().next_layer(), results.begin(), results.end(),
      // capture shared_ptr to keep the instance
      // capture this to access protected member
      [ connection = share_this<WssConnection>(), this ](
          const boost::system::error_code &error,
          tcp::resolver::iterator) mutable {
        if (is_destroyed()) return;

        if (error != boost::system::errc::success) {
          destroy_in_strand(std::move(connection));
          // TODO: log or return the error?
          return;
        }

        // ssl handshake
        _socket.next_layer().async_handshake(
            ssl::stream_base::client,
            [ connection = connection, this ](boost::system::error_code ec) {

              if (ec != boost::system::errc::success) {
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
                  LOG_ERROR(Logger::_(),
                            LOG << "Client websocket handshake failed:"
                                << error.message());
                  return;
                }

                start_client_f0();

                WssConnection::start_read(std::move(connection));

              }); // websocket handshake

            }); // ssl handshake

      });  // async_connect

  // use half of the reconnection time to resolve host
  start_deadline_timer((reconnect_interval >> 1) + 1);
}

}  // namespace dsa

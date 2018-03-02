#include "dsa_common.h"

#include "ws_client_connection.h"

#include <boost/asio/connect.hpp>

#include "module/logger.h"

namespace dsa {

WsClientConnection::WsClientConnection(bool is_secured, LinkStrandRef &strand,
                                       const string_ &dsid_prefix,
                                       const string_ &host, uint16_t port)
    : WsConnection(strand, dsid_prefix),
      _is_secured(is_secured),
      _tcp_socket(strand->get_io_context()),
      _ssl_context(boost::asio::ssl::context::sslv23),
      _hostname(host),
      _port(port) {
  if (_is_secured) {
    boost::system::error_code error;
    _ssl_context.load_verify_file("certificate.pem", error);
    if (error) {
      LOG_FATAL(__FILENAME__, LOG << "Client failed to verify SSL certificate");
    }

    _websocket =
        std::make_unique<Websocket>(std::move(_tcp_socket), _ssl_context);
  } else {
    _websocket = std::make_unique<Websocket>(std::move(_tcp_socket));
  }
}

void WsClientConnection::connect(size_t reconnect_interval) {
  // connect to server
  using tcp = boost::asio::ip::tcp;
  tcp::resolver resolver(_strand->get_io_context());

  tcp::resolver::results_type results =
      resolver.resolve(tcp::resolver::query(_hostname, std::to_string(_port)));

  if (!_is_secured) {
    // TODO: timeout
    LOG_FINE(__FILENAME__,
             LOG << "WS client connecting to " << _hostname << ":" << _port);

    boost::asio::async_connect(
        _websocket->stream().next_layer(), results.begin(), results.end(),
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
          _websocket->stream().async_handshake(_hostname, "/", [
            connection = connection, this
          ](const boost::system::error_code &error) mutable {
            if (is_destroyed()) return;
            if (error != boost::system::errc::success) {
              destroy_in_strand(std::move(connection));
              LOG_ERROR(__FILENAME__,
                        LOG << "Client websocket handshake failed");
              return;
            }

            start_client_f0();

            WsConnection::start_read(std::move(connection));

          });
        });
  } else {
    LOG_FINE(
        __FILENAME__,
        LOG << "WS secure client connecting to " << _hostname << ":" << _port);

    std::lock_guard<std::mutex> lock(_mutex);
    boost::asio::async_connect(
        _websocket->secure_stream().next_layer().next_layer(), results.begin(),
        results.end(),
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

          std::lock_guard<std::mutex> lock(_mutex);
          // ssl handshake
          _websocket->secure_stream().next_layer().async_handshake(
              ssl::stream_base::client,
              [ connection = connection, this ](boost::system::error_code ec) {
                if (ec != boost::system::errc::success) {
                  destroy_in_strand(std::move(connection));
                  // TODO: log or return the error?
                  return;
                }

                std::lock_guard<std::mutex> lock(_mutex);
                // websocket handshake
                _websocket->secure_stream().async_handshake(_hostname, "/", [
                  connection = connection, this
                ](const boost::system::error_code &error) mutable {
                  if (is_destroyed()) return;
                  if (error != boost::system::errc::success) {
                    destroy_in_strand(std::move(connection));
                    LOG_ERROR(__FILENAME__,
                              LOG << "Client websocket handshake failed:"
                                  << error.message());
                    return;
                  }

                  start_client_f0();

                  WsConnection::start_read(std::move(connection));

                });  // websocket handshake

              });  // ssl handshake

        });  // async_connect
  }

  // use half of the reconnection time to resolve host
  start_deadline_timer((reconnect_interval >> 1) + 1);
}

}  // namespace dsa

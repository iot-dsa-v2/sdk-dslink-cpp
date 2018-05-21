#include "dsa_common.h"

#include "module/logger.h"
#include "util/certificate.h"
#include "ws_client_connection.h"

namespace dsa {

WsClientConnection::WsClientConnection(bool is_secured,
                                       const SharedLinkStrandRef &strand,
                                       const string_ &dsid_prefix,
                                       const string_ &host, uint16_t port)
    : WsConnection(strand, dsid_prefix),
      _tcp_socket(strand->get_io_context()),
      _ssl_context(boost::asio::ssl::context::sslv23),
      _hostname(host),
      _port(port) {
  if (is_secured) {
    boost::system::error_code error_code;
    load_root_certificate(_ssl_context, error_code);

    _websocket =
        std::make_unique<Websocket>(std::move(_tcp_socket), _ssl_context);
  } else {
    _websocket = std::make_unique<Websocket>(std::move(_tcp_socket));
  }
}

void WsClientConnection::connect(size_t reconnect_interval) {
  // connect to server
  using tcp = boost::asio::ip::tcp;
  tcp::resolver resolver(_shared_strand->get_io_context());

  tcp::resolver::results_type results =
      resolver.resolve(tcp::resolver::query(_hostname, std::to_string(_port)));

  LOG_FINE(__FILENAME__,
           LOG << "WS client connecting to " << _hostname << ":" << _port);

  _websocket->async_connect(
      _hostname, results,
      CAST_LAMBDA(
          Websocket::ConnectCallback)[this,
                                      connection = share_this<WsConnection>()](
          const boost::system::error_code &err) {
        if (is_destroyed()) return;
        if (err != boost::system::errc::success) {
          destroy_in_strand(std::move(connection));
          // TODO: log or return the error?
          return;
        }
        start_client_f0();

        WsConnection::start_read(std::move(connection));
      });

  // use half of the reconnection time to resolve host
  start_deadline_timer((reconnect_interval >> 1) + 1);
}

}  // namespace dsa

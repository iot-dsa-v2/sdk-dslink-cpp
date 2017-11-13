#include "dsa_common.h"

#include "http_client_connection.h"

#include "module/logger.h"

namespace dsa {
HttpClientConnection::HttpClientConnection(LinkStrandRef &strand,
                                   const string_ &dsid_prefix,
                                   const string_ &tcp_host, uint16_t tcp_port)
    : HttpConnection(strand, dsid_prefix),
      _hostname(tcp_host),
      _port(tcp_port) {}

void HttpClientConnection::connect(size_t reconnect_interval) {
  // connect to server
  using tcp = boost::asio::ip::tcp;
  tcp::resolver resolver(_strand->get_io_service());
  // TODO: timeout
  LOG_INFO(_strand->logger(),
           LOG << "TCP client connecting to " << _hostname << ":" << _port);
  _socket.async_connect(
      *resolver.resolve(tcp::resolver::query(_hostname, std::to_string(_port))),
      // capture shared_ptr to keep the instance
      // capture this to access protected member
      [ connection = share_this<HttpConnection>(),
        this ](const boost::system::error_code &error) mutable {
        if (is_destroyed()) return;
	/* TODO
        if (error != boost::system::errc::success) {
          TcpConnection::destroy_in_strand(std::move(connection));
          // TODO: log or return the error?
          return;
        }
	*/

        start_client_f0();

        HttpConnection::start_read(std::move(connection));
      });
  start_deadline_timer(reconnect_interval);
}

}  // namespace dsa

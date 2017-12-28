#include "dsa_common.h"

#include <boost/asio/connect.hpp>

#include "stcp_client_connection.h"
#include "module/logger.h"

namespace dsa {

StcpClientConnection::StcpClientConnection(LinkStrandRef& strand,
                                           boost::asio::ssl::context& context,
                                           const string_& dsid_prefix,
                                           const string_& tcp_host,
                                           uint16_t tcp_port)
    : StcpConnection(strand, context, dsid_prefix),
      _hostname(tcp_host),
      _port(tcp_port) {
  _socket.set_verify_mode(boost::asio::ssl::verify_peer);
  _socket.set_verify_callback(
      [this](bool preverified,
             boost::asio::ssl::verify_context& context) -> bool {
        return verify_certificate(preverified, context);
      });
}

void StcpClientConnection::connect(size_t reconnect_interval) {
  // connect to server
  using tcp = boost::asio::ip::tcp;
  tcp::resolver resolver(_strand->get_io_context());
  LOG_INFO(_strand->logger(),
           LOG << "TCP client connecting to " << _hostname << ":" << _port);

  tcp::resolver::results_type results =
      resolver.resolve(tcp::resolver::query(_hostname, std::to_string(_port)));
  boost::asio::async_connect(
      _socket.lowest_layer(), results.begin(), results.end(),
      // capture shared_ptr to keep the instance
      // capture this to access protected member
      [ connection = share_this<StcpConnection>(), this ](
          const boost::system::error_code& error,
          tcp::resolver::iterator) mutable {
        if (is_destroyed()) return;
        /* TODO
        if (error != boost::system::errc::success) {
          TcpConnection::destroy_in_strand(std::move(connection));
          // TODO: log or return the error?
          return;
        }
        */

        boost::system::error_code ec;
        _socket.handshake(boost::asio::ssl::stream_base::client, ec);

        if (ec) {
          LOG_FATAL(LOG << "client handshake failed");
        }

        start_client_f0();

        StcpConnection::start_read(std::move(connection));
      });
  // use half of the reconnection time to resolve host
  start_deadline_timer((reconnect_interval >> 1) + 1);
}

bool StcpClientConnection::verify_certificate(
    bool preverified, boost::asio::ssl::verify_context& context) {
  // The verify callback can be used to check whether the certificate that is
  // being presented is valid for the peer. For example, RFC 2818 describes
  // the steps involved in doing this for HTTPS. Consult the OpenSSL
  // documentation for more details. Note that the callback is called once
  // for each certificate in the certificate chain, starting from the root
  // certificate authority.

  // In this example we will simply print the certificate's subject name.
  char subject_name[256];
  X509* cert = X509_STORE_CTX_get_current_cert(context.native_handle());
  X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
  //    std::cout << "Verifying " << subject_name << "\n";

  return preverified;
}

}  // namespace dsa

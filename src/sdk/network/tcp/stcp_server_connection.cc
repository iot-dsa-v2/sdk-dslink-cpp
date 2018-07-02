#include "dsa_common.h"

#include <boost/bind.hpp>
#include "module/logger.h"
#include "stcp_server_connection.h"

#define DEBUG 0

namespace dsa {

StcpServerConnection::StcpServerConnection(const SharedLinkStrandRef &strand,
                                           const string_ &dsid_prefix,
                                           const string_ &path)
    : StcpConnection(strand, dsid_prefix, path),
      _context(boost::asio::ssl::context::sslv23)

{
  _context.set_options(boost::asio::ssl::context::default_workarounds |
                       boost::asio::ssl::context::no_sslv2);
  _context.set_password_callback(
      boost::bind(&StcpServerConnection::get_password, this));

  boost::system::error_code error_code;
  load_server_certificate(_context, error_code);

  _socket = make_shared_<ssl_socket>(strand->get_io_context(), _context);
}

void StcpServerConnection::accept() {
  _socket->async_handshake(boost::asio::ssl::stream_base::server,
                           [this, sthis = shared_from_this()](
                               const boost::system::error_code &error) -> void {
                             return handle_handshake(error);
                           });
}

void StcpServerConnection::handle_handshake(
    const boost::system::error_code &error) {
  if (error != boost::system::errc::success) {
    LOG_ERROR(__FILENAME__, LOG << "Server SSL handshake failed: "
                                << error.message() << "\n");
  } else {
    {
      std::lock_guard<std::mutex> lock(mutex);
      on_read_message = [this](MessageRef message) {
        on_receive_f0(std::move(message));
      };
    }

    StcpConnection::start_read(share_this<StcpServerConnection>());
    start_deadline_timer(15);
  }
}

std::string StcpServerConnection::get_password() const { return ""; }

}  // namespace dsa

#include "dsa_common.h"

#include <boost/asio/strand.hpp>
#include <boost/beast/websocket.hpp>

#include "http_connection.h"
#include "network/connection.h"
#include "web_server.h"

namespace websocket =
    boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace dsa {

HttpConnection::HttpConnection(WebServer& web_server, bool is_secured)
    : _web_server(web_server),
      _socket(_web_server.io_service()),
      _is_secured(is_secured),
      _context(boost::asio::ssl::context::sslv23) {
  if (!_is_secured) return;

  try {
    _context.set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2);
    _context.set_password_callback(
        [](std::size_t, boost::asio::ssl::context_base::password_purpose) {
          return "";
        });

    _context.use_certificate_chain_file("certificate.pem");
    _context.use_private_key_file("key.pem", boost::asio::ssl::context::pem);

  } catch (boost::system::system_error& e) {
    LOG_ERROR(Logger::_(), LOG << "SSL context setup error: " << e.what());
    return;
  }
}

void HttpConnection::accept() {
  if (!_is_secured) {
    // Read a request
    boost::beast::http::async_read(
        _socket, _buffer, _req,
        // TODO: run within the strand?
        [ this, sthis = shared_from_this() ](
            const boost::system::error_code& error, size_t bytes_transferred) {

          // TODO: check error/termination conditions

          if (websocket::is_upgrade(_req)) {
            // call corresponding server's callback
            //          TODO - temporary fix for issue on Windowns platform
            //          _connection =
            //          _web_server.ws_handler(_req.target().to_string())(
            _connection = _web_server.ws_handler("/")(
                _web_server, std::move(_socket), std::move(_req));
          }
          return;
        });  // async_read
  } else {
  }
}

void HttpConnection::destroy() {
  if (_connection != nullptr) {
    _connection->destroy();
  }
}
}  // namespace dsa

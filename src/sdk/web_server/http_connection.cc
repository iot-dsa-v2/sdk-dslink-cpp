#include "dsa_common.h"

#include <boost/asio/strand.hpp>
#include <boost/beast/websocket.hpp>

#include "http_connection.h"
#include "web_server.h"

namespace websocket =
    boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace dsa {

HttpConnection::HttpConnection(WebServer& web_server)
    : _web_server(web_server), _socket(_web_server.io_service()) {}

void HttpConnection::accept() {
  // Read a request
  boost::beast::http::async_read(
      _socket, _buffer, _req,
      // TODO: run within the strand?
      [ this, sthis = shared_from_this() ](
          const boost::system::error_code& error, size_t bytes_transferred) {

        // TODO: check error/termination conditions

        if (websocket::is_upgrade(_req)) {
          // call corresponding server's callback
          _web_server.ws_handler(_req.target().to_string())(
              _web_server, std::move(_socket), std::move(_req));
        }
        return;
      });  // async_read
}

}  // namespace dsa

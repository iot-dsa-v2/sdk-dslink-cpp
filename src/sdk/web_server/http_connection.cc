#include "dsa_common.h"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>
#include <regex>

#include "crypto/misc.h"
#include "http_connection.h"
#include "network/connection.h"
#include "web_server.h"
#include "websocket_connection.h"

namespace websocket = boost::beast::websocket;
namespace http = boost::beast::http;

namespace dsa {

void HttpConnection::check_deadline() {
  auto self = shared_from_this();

  deadline_.async_wait([self](boost::beast::error_code ec) {
    if (!ec) {
      self->_socket.close(ec);
    }
  });
}

HttpConnection::HttpConnection(WebServer& web_server)
    : _web_server(web_server),
      _socket(_web_server.io_service()),
      deadline_{_socket.get_executor().context(), std::chrono::seconds(60)} {}

void HttpConnection::accept() {
  boost::beast::http::async_read(
      _socket, _buffer, _req,
      // TODO: run within the strand?
      [ this, sthis = shared_from_this() ](
          const boost::system::error_code& error, size_t bytes_transferred) {

        // TODO: check error/termination conditions

        if (websocket::is_upgrade(_req)) {
          // call corresponding server's callback
          //          TODO - temporary fix for issue on Windowns platform
          _connection = std::make_shared<WebsocketConnection>(
              _web_server, std::move(_socket));
          _connection->accept(std::move(_req));
          return;
        }
        _web_server.http_handler(_req.target().to_string())(
            _web_server, std::move(_socket), std::move(_req));
        return;
      });  // async_read
   check_deadline();
}

void HttpConnection::destroy() {
  /*
    if(_connection != nullptr) {
      _connection->destroy();
    }
  */
}
}  // namespace dsa

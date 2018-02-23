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
#include "http_request.h"

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
  reset_parser();
  boost::beast::http::async_read(
      _socket, _buffer, *_parser,
      // TODO: run within the strand?
      [ this, sthis = shared_from_this() ](
          const boost::system::error_code& error, size_t bytes_transferred) {

        // TODO: check error/termination conditions

        if (_parser->upgrade()) {
          //          TODO - temporary fix for issue on Windows platform
          _connection = std::make_shared<WebsocketConnection>(
              _web_server, std::move(_socket));
          _connection->accept(std::move(_parser->get()));
          return;
        }
        auto _target = _parser->get().target().to_string();
        _req = std::make_shared<HttpRequest>(_web_server, std::move(_socket), std::move(_parser->get()));
        _web_server.http_handler(_target)(_web_server, std::move(*_req));
        return;
      });
   check_deadline();
}

void HttpConnection::reset_parser() {
  _parser.emplace(std::piecewise_construct, std::make_tuple(),
                  std::make_tuple(_alloc));
}

void HttpConnection::destroy() {
  /*
    if(_connection != nullptr) {
      _connection->destroy();
    }
  */
}
}  // namespace dsa

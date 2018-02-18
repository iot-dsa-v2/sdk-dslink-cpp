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
      _websocket(_web_server.io_service(), _web_server.ssl_context()),
      _is_secured(is_secured) {}

void HttpConnection::accept() {
  if (!_is_secured) {
    // Read a request
    boost::beast::http::async_read(
        _websocket.socket(), _buffer, _req,
        // TODO: run within the strand?
        [ this, sthis = shared_from_this() ](
            const boost::system::error_code& error, size_t bytes_transferred) {

          // TODO: check error/termination conditions

          if (websocket::is_upgrade(_req)) {
            // call corresponding server's callback
            //          TODO - temporary fix for issue on Windowns platform
            //          _connection =
            //          _web_server.ws_handler(_req.target().to_string())(
            _connection = _web_server.ws_handler("/")(_web_server, _websocket,
                                                      std::move(_req));
          }
          return;
        });  // async_read
  } else {
    _websocket.secure_stream().next_layer().async_handshake(
        ssl::stream_base::server, [ this, sthis = shared_from_this() ](
                                      const boost::system::error_code& error) {

          // Read a request
          boost::beast::http::async_read(
              _websocket.secure_stream().next_layer(), _buffer, _req,
              // TODO: run within the strand?
              [ this, sthis = sthis ](const boost::system::error_code& error,
                                      size_t bytes_transferred)
                  ->void {

                    // TODO: check error/termination conditions

                    if (websocket::is_upgrade(_req)) {
                      // call corresponding server's callback
                      //          TODO - temporary fix for issue on Windowns
                      //          platform
                      //          _connection =
                      //          _web_server.ws_handler(_req.target().to_string())(

                      _websocket.set_secure_stream();
                      _connection = _web_server.ws_handler("/")(
                          _web_server, _websocket, std::move(_req));
                    }
                    return;
                  });  // async_read
          return;
        });  // async_handshake
  }          // else
}

void HttpConnection::destroy() {
  if (_connection != nullptr) {
    _connection->destroy();
  }
}
}  // namespace dsa

#include "dsa_common.h"

#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <regex>

#include "crypto/misc.h"
#include "http_connection.h"
#include "network/connection.h"
#include "web_server.h"

namespace websocket = boost::beast::websocket;

namespace dsa {

void HttpConnection::check_deadline() {
  auto self = shared_from_this();

  deadline_.async_wait([self](boost::beast::error_code ec) {
    if (!ec) {
      self->_socket.close(ec);
    }
  });
}

HttpConnection::HttpConnection(WebServer& web_server, bool is_secured)
    : _web_server(web_server),
      _socket(_web_server.io_service()),
      deadline_{_socket.get_executor().context(), std::chrono::seconds(60)},
      _is_secured(is_secured) {}

void HttpConnection::accept() {
  if (!_is_secured) {
    _websocket = std::make_unique<Websocket>(std::move(_socket));

    // Read a request
    boost::beast::http::async_read(
        _websocket->stream().next_layer(), _buffer, _req,
        // TODO: run within the strand?
        [ this, sthis = shared_from_this() ](
            const boost::system::error_code& error, size_t bytes_transferred) {

          // check error
          if (error != boost::system::errc::success) {
            LOG_ERROR(__FILENAME__,
                      LOG << "HTTP read failed: " << error.message());
            // TODO: send error response
          }

          if (websocket::is_upgrade(_req)) {
            _websocket->stream().async_accept(_req, [
              this, sthis = sthis
            ](const boost::system::error_code& error) {

              // check error
              if (error != boost::system::errc::success) {
                LOG_ERROR(
                    __FILENAME__,
                    LOG << "websocket handshake failed: " << error.message());
                // TODO: send error response
              }

              // call corresponding server's callback
              //          TODO - temporary fix for issue on Windowns
              //          platform
              //          _connection =
              //          _web_server.ws_handler(_req.target().to_string())(

              _websocket->set_websocket();
              _connection = _web_server.ws_handler("/")(
                  _web_server, std::move(_websocket),
                  std::move(_req));
              return;
            });  // async_accept
          } else {
	      // TODO: http code
            return;
          }
        });  // async_read
    // check_deadline();
  } else {
    _websocket = std::make_unique<Websocket>(std::move(_socket),
                                             _web_server.ssl_context());
    // std::lock_guard<std::mutex> lock(_mutex);
    _websocket->secure_stream().next_layer().async_handshake(
        ssl::stream_base::server, [ this, sthis = shared_from_this() ](
                                      const boost::system::error_code& error) {
          // Read a request
          //	  std::lock_guard<std::mutex> lock(_mutex);
          boost::beast::http::async_read(
              _websocket->secure_stream().next_layer(), _buffer, _req,

              // TODO: run within the strand?
              [ this, sthis = sthis ](const boost::system::error_code& error,
                                      size_t bytes_transferred)
                  ->void {

                    // check error
                    if (error != boost::system::errc::success) {
                      LOG_ERROR(
                          __FILENAME__,
                          LOG << "HTTPS read failed: " << error.message());
                      // TODO: send error response
                    }

                    if (websocket::is_upgrade(_req)) {
                      // call corresponding server's callback
                      //          TODO - temporary fix for issue on Windowns
                      //          platform
                      //          _connection =
                      //          _web_server.ws_handler(_req.target().to_string())(

                      _websocket->secure_stream().async_accept(_req, [
                        this, sthis = sthis
                      ](const boost::system::error_code& error) {

                        // check error
                        if (error != boost::system::errc::success) {
                          LOG_ERROR(__FILENAME__,
                                    LOG << "Secure websocket handshake failed: "
                                        << error.message());
                          // TODO: send error response
                        }

                        // call corresponding server's callback
                        //          TODO - temporary fix for issue on
                        //          Windowns
                        //          platform
                        //          _connection =
                        //          _web_server.ws_handler(_req.target().to_string())(
                        _websocket->set_websocket();
                        _connection = _web_server.ws_handler("/")(
                            _web_server, std::move(_websocket),
                            std::move(_req));
                        return;
                      });  // async_accept
                      return;
                    } else {
                      // TODO: http code
                      return;
                    }
                  });  // async_read
          // check_deadline();
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

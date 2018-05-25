#include "dsa_common.h"

#include <boost/algorithm/string.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <regex>

#include "crypto/misc.h"
#include "http_connection.h"
#include "network/connection.h"
#include "network/ws/ws_callback.h"
#include "web_server.h"

namespace websocket = boost::beast::websocket;

namespace dsa {

HttpConnection::HttpConnection(WebServer& web_server, bool is_secured)
    : _web_server(web_server),
      _socket(_web_server.io_service()),
      _is_secured(is_secured) {}
struct V1Uri {
  string_ path;
  string_ dsid;
  string_ token;
  string_ auth;
};
V1Uri parseUri(const string_& str) {
  V1Uri result;
  if (str.size() < 3) {
    return result;
  }
  std::vector<string_> splitted;
  boost::algorithm::split(splitted, str, boost::algorithm::is_any_of("?&="));

  result.path = std::move(splitted[0]);
  for (int i = 0; i + 1 < splitted.size(); ++i) {
    string_& current = splitted[i];
    if (current == "dsId") {
      result.dsid = std::move(splitted[i + 1]);
      ++i;
    } else if (current == "token") {
      result.token = std::move(splitted[i + 1]);
      ++i;
    } else if (current == "auth") {
      result.auth = std::move(splitted[i + 1]);
      ++i;
    }
  }
  return std::move(result);
}

void HttpConnection::accept() {
  if (_is_secured) {
    _websocket =
        make_shared_<Websocket>(std::move(_socket), _web_server.ssl_context());
  } else {
    _websocket = make_shared_<Websocket>(std::move(_socket));
  }
  _websocket->http_async_read(
      _buffer, _req,
      CAST_LAMBDA(Websocket::Callback)[this, sthis = shared_from_this()](
          const boost::system::error_code& error,
          size_t bytes_transferred) mutable {

        // check error
        if (error != boost::system::errc::success) {
          LOG_ERROR(__FILENAME__,
                    LOG << "HTTP read failed: " << error.message());
          // TODO: send error response
          // TODO cleanup;
          return;
        }

        if (websocket::is_upgrade(_req)) {
          _websocket->async_accept(
              _req,
              CAST_LAMBDA(
                  Websocket::ConnectCallback)[this, sthis = std::move(sthis)](
                  const boost::system::error_code& error) {

                // check error
                if (error != boost::system::errc::success) {
                  LOG_ERROR(__FILENAME__, LOG << "websocket handshake failed: "
                                              << error.message());
                  // TODO: send error response
                  // TODO cleanup;
                  return;
                }

                V1Uri uri = parseUri(_req.target().to_string());

                // call corresponding server's callback
                //          TODO - temporary fix for issue on Windowns
                //          platform
                //          _connection =
                //          _web_server.ws_handler(_req.target().to_string())(

                _websocket->set_websocket();

                if (_web_server._v1_ws_callback != nullptr &&
                    uri.path == "/ws") {
                  _web_server._v1_ws_callback(std::move(_websocket), uri.dsid,
                                              uri.auth);
                } else {
                  _connection = make_shared_<WsServerConnection>(
                      std::move(_websocket), _web_server.get_shared_strand());
                  std::dynamic_pointer_cast<WsConnection>(_connection)
                      ->accept();
                }

                return;
              });  // async_accept
        } else {
          V1Uri uri = parseUri(_req.target().to_string());
          auto body = _req.body();

          if (_web_server._v1_conn_callback != nullptr && uri.path == "/conn") {
            _web_server._v1_conn_callback(
                uri.dsid, uri.token, body,
                [ this, sthis = std::move(sthis) ](const string_& response) {
                  auto resp = make_shared_<http::response<http::string_body>>(
                      http::status::ok, _req.version());

                  resp->body() = response;
                  resp->prepare_payload();

                  // get raw pointer before it's captured
                  auto& raw_resp = *resp;
                  _websocket->http_async_write(
                      raw_resp,
                      CAST_LAMBDA(
                          Websocket::Callback)[this, sthis = std::move(sthis),
                                               resp = std::move(resp)](
                          const boost::system::error_code& err,
                          size_t transferred) { _websocket->destroy(); });
                });

            return;
          }
          _websocket->destroy();
          return;
        }
      });  // async_read
}  // namespace dsa

void HttpConnection::destroy() {
  if (_connection != nullptr) {
    _connection->destroy();
  }
}
}  // namespace dsa

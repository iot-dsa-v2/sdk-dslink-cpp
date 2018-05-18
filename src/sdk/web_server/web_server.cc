#include "dsa_common.h"

#include "web_server.h"

#include "module/default/simple_security.h"
#include "module/default/simple_session_manager.h"
#include "util/app.h"
#include "util/certificate.h"

#include <iostream>

namespace dsa {

WebServer::WebServer(App& app, const LinkStrandRef& strand)
    : _io_service(app.io_service()),
      _ssl_context{boost::asio::ssl::context::sslv23},
      _shared_strand(share_strand_(strand)) {
  try {
    _ssl_context.set_options(boost::asio::ssl::context::default_workarounds |
                             boost::asio::ssl::context::no_sslv2);
    _ssl_context.set_password_callback(
        [](std::size_t, boost::asio::ssl::context_base::password_purpose) {
          return "";
        });

    boost::system::error_code error_code;
    if (!load_server_certificate(_ssl_context, error_code)) {
      return;
    }
  } catch (boost::system::system_error& e) {
    LOG_ERROR(__FILENAME__, LOG << "SSL context setup error: " << e.what());
    return;
  }
}

void WebServer::initV1Connection(V1ConnCallback&& conn_callback,
                                 V1WsCallback&& ws_callback) {
  if (_v1_conn_callback == nullptr && _v1_ws_callback == nullptr &&
      conn_callback != nullptr && ws_callback != nullptr) {
    _v1_conn_callback = std::move(conn_callback);
    _v1_ws_callback = std::move(ws_callback);
  }
}
void WebServer::listen(uint16_t port) {
  _port = port;
  _listener = make_shared_<Listener>(*this, port, false);
}

void WebServer::secure_listen(uint16_t port) {
  _secure_port = port;
  _secure_listener = make_shared_<Listener>(*this, port);
}

void WebServer::start() {
  if (_listener != nullptr) {
    _listener->run();
  }
  if (_secure_listener != nullptr) {
    _secure_listener->run();
  }
}

void WebServer::destroy() {
  if (_listener != nullptr) {
    _listener->destroy();
  }
  if (_secure_listener != nullptr) {
    _secure_listener->destroy();
  }
  if (_v1_conn_callback != nullptr) {
    _v1_conn_callback = nullptr;
  }
  if (_v1_ws_callback != nullptr) {
    _v1_ws_callback = nullptr;
  }
}

WebServer::~WebServer() = default;

}  // namespace dsa

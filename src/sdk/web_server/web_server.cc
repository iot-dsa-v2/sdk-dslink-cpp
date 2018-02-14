#include "dsa_common.h"

#include "web_server.h"

#include "util/app.h"

#include <iostream>

namespace dsa {

WebServer::WebServer(App& app)
  : _io_service(app.io_service()) {}

void WebServer::listen(uint16_t port) {
  _port = port;
  _listener = make_shared_<Listener>(*this, port);
}

void WebServer::start() {
  if (_listener != nullptr) {
    _listener->run();
  }
}

void WebServer::add_ws_handler(const string_& path, WsCallback&& callback) {
  if (!_ws_callback_map.count(path)) {
    _ws_callback_map.insert(WsCallbackPair(path, std::move(callback)));
  }
  // TODO: report error/warning otherwise
}

WebServer::WsCallback& WebServer::ws_handler(const string_& path) {
  if (_ws_callback_map.count(path)) {
    return _ws_callback_map.at(path);
  }

  uint16_t error_code = 404;
  static WebServer::WsCallback error_callback = [error_code](
      WebServer& web_server, boost::asio::ip::tcp::socket&& socket,
      boost::beast::http::request<boost::beast::http::string_body> req) {

    ErrorCallback error_callback_detail(error_code);
    error_callback_detail(web_server, std::move(socket), std::move(req));

    return nullptr;
  };

  return error_callback;
}

void WebServer::destroy() {
  if (_listener != nullptr) {
    _listener->destroy();
  }
  // TODO - hard coded for now
  if (_ws_callback_map.count("/")) {
    delete &_ws_callback_map.at("/");
    _ws_callback_map.erase("/");
  }
}

WebServer::~WebServer() = default;

}  // namespace dsa

#include "dsa_common.h"

#include "web_server.h"

#include "util/app.h"
#include "util/enable_shared.h"

namespace dsa {

WebServer::WebServer(App& app) : _io_service(app.io_service()) {}

void WebServer::listen(uint16_t port) {
  _port = port;
  _listener = std::shared_ptr<Listener>(new Listener(*this, _io_service, port));
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

WebServer::WsCallback&& WebServer::ws_handler(const string_& path) {
  if (_ws_callback_map.count(path)) {
    return std::move(_ws_callback_map.at(path));
  }
  return std::move(nullptr);
}

WebServer::~WebServer() = default;

}  // namespace dsa

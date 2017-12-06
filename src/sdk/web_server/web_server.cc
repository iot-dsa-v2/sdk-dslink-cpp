#include "dsa_common.h"

#include "web_server.h"

#include "util/app.h"
#include "util/enable_shared.h"

namespace dsa {

WebServer::WebServer(App& app)
  : _io_service(app.io_service()) {}

void WebServer::listen(uint16_t port) {
  _port = port;
  _listener = std::unique_ptr<Listener>(new Listener(_io_service, port));
}

void WebServer::start() {
  if (_listener != nullptr) {
    _listener->run();
  }
}

WebServer::~WebServer() = default;

}  // namespace dsa

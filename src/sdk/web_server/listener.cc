#include "dsa_common.h"

#include "listener.h"

#include "http_connection.h"
#include "util/enable_shared.h"
#include "web_server.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;

Listener::Listener(WebServer& web_server, uint16_t port, bool is_secured)
    : _web_server(web_server), _is_secured(is_secured) {
  try {
    _acceptor = std::make_shared<tcp::acceptor>(
        _web_server.io_service(),
        // TODO - server port
        // tcp:v6() already covers both ipv4 and ipv6
        tcp::endpoint(tcp::v6(), port));
  } catch (boost::system::system_error& e) {
    switch (e.code().value()) {
      case boost::asio::error::address_in_use:
      case boost::asio::error::access_denied:
        LOG_FATAL(__FILENAME__,
                  LOG << "Bind Error: port " << port << " is already in use\n");
        break;
      default:
        LOG_FATAL(__FILENAME__, LOG << "Bind Error: " << e.what() << "\n");
    }
  }
}

void Listener::run() {
  std::lock_guard<std::mutex> lock(_mutex);
  _next_connection = make_shared_<HttpConnection>(_web_server, _is_secured);

  _acceptor->async_accept(_next_connection->socket(), [
    this, sthis = shared_from_this()
  ](const boost::system::error_code& error) { accept_loop(error); });
}

void Listener::accept_loop(const boost::system::error_code& error) {
  if (!error) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_destroyed) return;

    _next_connection->accept();

    _next_connection = make_shared_<HttpConnection>(_web_server, _is_secured);
    _acceptor->async_accept(_next_connection->socket(), [
      this, sthis = shared_from_this()
    ](const boost::system::error_code& error) { accept_loop(error); });
  } else {
    destroy();
  }
}

void Listener::destroy() {
  std::lock_guard<std::mutex> lock(_mutex);
  if (_destroyed) return;
  _destroyed = true;
  if (_acceptor->is_open()) {
    _acceptor->close();
  }
  _next_connection->destroy();
  _next_connection.reset();
}

Listener::~Listener() {}
}  // namespace dsa

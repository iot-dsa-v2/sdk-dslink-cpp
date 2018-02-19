#include "dsa_common.h"

#include "listener.h"

#include "http_connection.h"
#include "util/enable_shared.h"
#include "web_server.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;

Listener::Listener(WebServer& web_server, uint16_t port)
    : _web_server(web_server),
      _acceptor(new tcp::acceptor(_web_server.io_service(),
                                  // TODO - server port
                                  // tcp:v6() already covers both ipv4 and ipv6
                                  tcp::endpoint(tcp::v6(), port))) {}

void Listener::run() {
  std::lock_guard<std::mutex> lock(_mutex);
  _next_connection = make_shared_<HttpConnection>(_web_server);

  _acceptor->async_accept(_next_connection->socket(), [
    this, sthis = shared_from_this()
  ](const boost::system::error_code& error) { accept_loop(error); });
}

void Listener::accept_loop(const boost::system::error_code& error) {
  if (!error) {
    std::lock_guard<std::mutex> lock(_mutex);
    if (_destroyed) return;

    _next_connection->accept();

    _next_connection = make_shared_<HttpConnection>(_web_server);
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

#include "dsa_common.h"

#include "web_server.h"

#include "core/app.h"

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

namespace dsa {

Listener::Listener(boost::asio::io_service& ios, tcp::endpoint endpoint,
                   std::string const& doc_root)
    : _acceptor(ios), _socket(ios), _doc_root(doc_root) {
  boost::system::error_code ec;

  // Open the acceptor
  _acceptor.open(endpoint.protocol(), ec);
  if (ec) {
    std::cout << ec << "open" << std::endl;
    return;
  }

  // Bind to the server address
  _acceptor.bind(endpoint, ec);
  if (ec) {
    std::cout << ec << "bind" << std::endl;
    return;
  }

  // Start listening for connections
  _acceptor.listen(boost::asio::socket_base::max_connections, ec);
  if (ec) {
    std::cout << ec << "listen" << std::endl;
    return;
  }
}

Listener::~Listener() {}

void Listener::start() {
  if (!_acceptor.is_open()) return;

  std::function<void(const boost::system::error_code&)> do_accept;
  do_accept = [this](const boost::system::error_code& ec) {
    if (ec) {
      std::cout << ec << "accept" << std::endl;
    } else {
      // Create the http_session and run it
      std::make_shared<HttpSession>(std::move(_socket), _doc_root)->start();
    }
  };

  _acceptor.async_accept(_socket, do_accept);
}

//-------------------------------------
//-------------------------------------
WebServer::WebServer(App& app) : _app(app) {}

void WebServer::start() {
  // create and launch listener
  // TODO
  auto const address = boost::asio::ip::address::from_string("0.0.0.0");
  auto const port = static_cast<unsigned short>(8080);
  std::string const doc_root = ".";

  std::make_shared<Listener>(_app.io_service(), tcp::endpoint{address, port},
                             doc_root)
      ->start();
}

void WebServer::destroy() {}

WebServer::~WebServer() {}

}  // namespace dsa

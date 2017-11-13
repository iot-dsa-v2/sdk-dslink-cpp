#include "dsa_common.h"

#include "web_server.h"

#include "util/app.h"
#include "util/enable_shared.h"

#include <algorithm>
#include <iostream>
#include <thread>
#include <vector>

namespace dsa {

void HttpSession::start() {
}

//-------------------------------------
//-------------------------------------
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
  do_accept = [&](const boost::system::error_code& ec) {
    if (ec) {
      std::cout << ec << "accept" << std::endl;
    } else {
      // Create the http_session and run it
      std::make_shared<HttpSession>(std::move(_socket), _doc_root)->start();
    }
    _acceptor.async_accept(_socket, do_accept);
  };

  _acceptor.async_accept(_socket, do_accept);
}

//-------------------------------------
//-------------------------------------
WebServer::WebServer(App& app) 
  : _io_service(&app.io_service()),
    _strand(make_shared_<boost::asio::io_service::strand>(*_io_service)) {}

void WebServer::start() {

  // TODO: config
  auto const address = boost::asio::ip::address::from_string("0.0.0.0");
  unsigned short const port = 8080;
  std::string const doc_root = ".";

  // _next_connection = make_shared_<HttpServerConnection>(_strand);
  //  make_shared_<HttpServerConnection>(_strand);

  LinkStrandRef ls_ref(new LinkStrand(_strand.get(), new ECDH()));
  auto http_server_connection = new HttpServerConnection(ls_ref);

  /*
  std::make_shared<Listener>(*_io_service, tcp::endpoint{address, port},
                             doc_root)
      ->start();
  */
}

void WebServer::destroy() {}

WebServer::~WebServer() {}

}  // namespace dsa

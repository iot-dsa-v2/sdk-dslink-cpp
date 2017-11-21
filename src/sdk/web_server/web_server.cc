#include "dsa_common.h"

#include "web_server.h"

#include "core/editable_strand.h"
#include "core/session_manager.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security_manager.h"
#include "util/app.h"
#include "util/enable_shared.h"

namespace dsa {

using tcp = boost::asio::ip::tcp;

WebServer::WebServer(App& app)
    : _io_service(&app.io_service()),
      _strand(make_shared_<boost::asio::io_service::strand>(*_io_service)),
      _acceptor(
		new tcp::acceptor(*_io_service,
// TODO - server port
#if defined(__CYGWIN__)
				  tcp::endpoint(tcp::v4(), 8080))) {
#else
                                  // tcp:v6() already covers both ipv4 and ipv6
                                  tcp::endpoint(tcp::v6(), 8080))) {
#endif

  // TODO -
  _port = 8080;
  _doc_root = ".";

  // start taking connections
  auto* config = new EditableStrand(_strand.get(), make_unique_<ECDH>());
  config->set_session_manager(make_ref_<SessionManager>(config));
  config->set_security_manager(make_ref_<SimpleSecurityManager>());
  config->set_logger(make_unique_<ConsoleLogger>());
  config->logger().level = Logger::WARN__;
  _link_strand.reset(config);

  LOG_INFO(_link_strand->logger(), LOG << "Bind to TCP server port: " << _port);
}

void WebServer::start() {
  _next_connection = make_shared_<WsServerConnection>(_link_strand);

  _acceptor->async_accept(_next_connection->socket(), [
    this, sthis = shared_from_this()
  ](const boost::system::error_code& error) { accept_loop(error); });
}

void WebServer::accept_loop(const boost::system::error_code& error) {
  if (!error) {
    _next_connection->accept();
    _next_connection = make_shared_<WsServerConnection>(_link_strand);
    _acceptor->async_accept(_next_connection->socket(), [
      this, sthis = shared_from_this()
    ](const boost::system::error_code& err) { accept_loop(err); });
  } else {
    destroy();
  }
}

void WebServer::destroy() {}

WebServer::~WebServer() {}

}  // namespace dsa

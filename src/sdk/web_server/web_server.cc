#include "dsa_common.h"

#include "web_server.h"

#include "util/app.h"
#include "util/certificate.h"
#include "module/default/simple_session_manager.h"
#include "module/default/simple_security.h"

#include <iostream>

namespace dsa {

WebServer::WebServer(App& app)
    : _io_service(app.io_service()),
      _ssl_context{boost::asio::ssl::context::sslv23} {
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
  // shared strand
#if 0
/*
  auto *strand =
        new boost::asio::io_service::strand(app.io_service(), make_unique_<ECDH>());
*/

          auto *strand = new EditableStrand(
              new boost::asio::io_service::strand(_io_service),
              make_unique_<ECDH>());

//      make_ref_<EditableStrand>(app.new_strand(), make_unique_<ECDH>());

//  strand->set_session_manager(make_ref_<SimpleSessionManager>(strand));

//  strand->set_security_manager(make_ref_<SimpleSecurityManager>());

//  strand->set_logger(make_unique_<ConsoleLogger>());
//  strand->logger().level = Logger::WARN__;

  LinkStrandRef link_strand;
  link_strand.reset(strand);
  _shared_strand = share_strand_(link_strand);
#endif
  auto strand =
      make_ref_<EditableStrand>(app.new_strand(), make_unique_<ECDH>());
  strand->set_session_manager(make_ref_<SimpleSessionManager>(strand));

  strand->set_client_manager(make_ref_<SimpleClientManager>());
  strand->set_authorizer(make_ref_<SimpleAuthorizer>(strand));

  _shared_strand = share_strand_(strand);
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
}

WebServer::~WebServer() = default;

}  // namespace dsa

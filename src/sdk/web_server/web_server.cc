#include "dsa_common.h"

#include "web_server.h"

#include "util/app.h"

#include <iostream>

namespace dsa {

WebServer::WebServer(App& app)
    : _io_service(app.io_service()),
      _context(boost::asio::ssl::context::sslv23) {
  try {
    _context.set_options(boost::asio::ssl::context::default_workarounds |
                         boost::asio::ssl::context::no_sslv2);
    _context.set_password_callback(
        [](std::size_t, boost::asio::ssl::context_base::password_purpose) {
          return "";
        });

    _context.use_certificate_chain_file("certificate.pem");
    _context.use_private_key_file("key.pem", boost::asio::ssl::context::pem);

  } catch (boost::system::system_error& e) {
    LOG_ERROR(Logger::_(), LOG << "Bind Error: " << e.what());
    return;
  }
}

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

boost::asio::ssl::context& WebServer::ssl_context() { return _context; }

WebServer::~WebServer() = default;

}  // namespace dsa

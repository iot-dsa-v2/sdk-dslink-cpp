#include "dsa_common.h"

#include "test_config.h"

#include "core/client.h"
#include "crypto/ecdh.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security.h"
#include "module/default/simple_session_manager.h"
#include "network/tcp/stcp_client_connection.h"
#include "network/tcp/tcp_client_connection.h"
#include "network/tcp/tcp_server.h"
#include "network/ws/ws_callback.h"
#include "network/ws/ws_client_connection.h"
#include "util/app.h"
#include "util/certificate.h"
#include "web_server/websocket.h"

#include "responder/model_base.h"
#include "responder/node_state_manager.h"

#include <boost/filesystem.hpp>
#include <boost/system/error_code.hpp>

namespace dsa {

TestConfig::TestConfig(std::shared_ptr<App> &app, bool async,
                       dsa::ProtocolType protocol)
    : WrapperStrand() {
  this->app = app;
  strand = EditableStrand::make_default(app);
  this->protocol = protocol;

  tcp_server_port = 0;
  tcp_secure_port = 0;

  std::vector<std::string> pem_files = {"key.pem", "certificate.pem"};
  namespace fs = boost::filesystem;
  for (auto pem_file : pem_files) {
    if (!fs::exists(pem_file)) {
      dsa::generate_certificate();
      break;
    }
  }
}

WrapperStrand TestConfig::get_client_wrapper_strand() {
  if (tcp_server_port == 0) {
    throw "There is no server to connect right now. Please create a server first";
  }

  WrapperStrand copy(*this);

  copy.tcp_server_port = 0;
  copy.tcp_host = "127.0.0.1";

  copy.strand = EditableStrand::make_default(app);

  switch (protocol) {
    case dsa::ProtocolType::PROT_DSS:
      copy.tcp_port = tcp_secure_port;
      copy.secure = true;
      break;
    case dsa::ProtocolType::PROT_WS:
      copy.ws_host = "127.0.0.1";
      // TODO: ws_port and ws_path
      copy.ws_port = 8080;
      copy.ws_path = "/";
      break;
    case dsa::ProtocolType::PROT_WSS:
      copy.ws_host = "127.0.0.1";
      // TODO: ws_port and ws_path
      copy.ws_port = 8443;
      copy.ws_path = "/";
      copy.secure = true;
      break;
    case dsa::ProtocolType::PROT_DS:
    default:
      copy.tcp_port = tcp_server_port;
  }

  copy.set_client_connection_maker();

  return std::move(copy);
}

ref_<DsLink> TestConfig::create_dslink(bool async) {
  if (tcp_server_port == 0) {
    throw "There is no server to connect right now. Please create a server first";
  }

  std::string address;

  switch (protocol) {
    case dsa::ProtocolType::PROT_DSS:
      address.assign(std::string("dss://127.0.0.1:") +
                     std::to_string(tcp_secure_port));
      break;
    case dsa::ProtocolType::PROT_WS:
      // TODO address.assign(std::string("ws://127.0.0.1:") +
      // std::to_string(ws_port));
      address.assign(std::string("ws://127.0.0.1:") + std::to_string(8080));
      break;
    case dsa::ProtocolType::PROT_WSS:
      // TODO address.assign(std::string("wss://127.0.0.1:") +
      // std::to_string(ws_port));
      address.assign(std::string("wss://127.0.0.1:") + std::to_string(8443));
      break;
    case dsa::ProtocolType::PROT_DS:
    default:
      address.assign(std::string("ds://127.0.0.1:") +
                     std::to_string(tcp_server_port));
  }

  const char *argv[] = {"./test", "-b", address.c_str()};
  int argc = 3;
  auto link = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0", app);
  static_cast<ConsoleLogger &>(Logger::_()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;
  return link;
}

std::shared_ptr<TcpServer> TestConfig::create_server() {
  auto tcp_server = std::make_shared<TcpServer>(*this);
  tcp_server_port = tcp_server->get_port();
  tcp_secure_port = tcp_server->get_secure_port();
  return tcp_server;
}

std::shared_ptr<WebServer> TestConfig::create_webserver() {
  shared_ptr_<WebServer> web_server = std::make_shared<WebServer>(*app);
  uint16_t http_port = 8080;
  web_server->listen(http_port);
  uint16_t https_port = 8443;
  web_server->secure_listen(https_port);

  WebServer::WsCallback *root_cb = new WebServer::WsCallback();
  *root_cb = [this](
      WebServer &web_server, std::unique_ptr<Websocket> &&websocket,
      http::request<request_body_t, http::basic_fields<alloc_t>> &&req) {
    DsaWsCallback dsa_ws_callback(strand);
    return dsa_ws_callback(web_server.io_service(), std::move(websocket),
                           std::move(req));
  };

  web_server->add_ws_handler("/", std::move(*root_cb));

  return web_server;
}

void destroy_client_in_strand(ref_<Client> &client) {
  client->get_strand().post([&client]() { client->destroy(); });
}

void destroy_dslink_in_strand(ref_<DsLink> &dslink) {
  dslink->strand->dispatch([&dslink]() { dslink->destroy(); });
}
}

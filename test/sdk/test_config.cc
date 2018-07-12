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
#include "util/string.h"
#include "web_server/socket.h"

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

  TestConfig::create_pem_files();
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
  shared_ptr_<WebServer> web_server = std::make_shared<WebServer>(*app, strand);
  uint16_t http_port = 8080;
  web_server->listen(http_port);
  uint16_t https_port = 8443;
  web_server->secure_listen(https_port);

  return web_server;
}

void destroy_client_in_strand(ref_<Client> &client) {
  client->get_strand().post([&client]() { client->destroy(); });
}

void destroy_dslink_in_strand(ref_<DsLink> &dslink) {
  dslink->strand->dispatch([&dslink]() { dslink->destroy(); });
}

void TestConfig::create_pem_files() {
  auto &config_bucket = Storage::get_config_bucket();
  string_ const cert =
      "-----BEGIN CERTIFICATE-----\n"
      "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\n"
      "-----END CERTIFICATE-----\n";

  string_to_storage(cert, "certificate.pem", config_bucket);

  string_ const key =
      "-----BEGIN RSA PRIVATE KEY-----\n"
      "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\n"
      "-----END RSA PRIVATE KEY-----\n";

  string_to_storage(key, "key.pem", config_bucket);
}
}  // namespace dsa

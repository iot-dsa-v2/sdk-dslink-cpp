#include "dsa_common.h"

#include "test_config.h"

#include "core/client.h"
#include "crypto/ecdh.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security_manager.h"
#include "module/default/simple_session_manager.h"
#include "network/tcp/tcp_client_connection.h"
#include "network/tcp/tcp_server.h"
#include "util/app.h"
#include "util/certificate.h"

#include "responder/model_base.h"
#include "responder/node_state_manager.h"

#include <boost/filesystem.hpp>

namespace dsa {

TestConfig::TestConfig(std::shared_ptr<App> &app, bool async)
    : WrapperStrand() {
  this->app = app;
  strand = EditableStrand::make_default(app);

  tcp_server_port = 0;
  // tcp_secure_port = 4128;

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
  copy.tcp_port = tcp_server_port;

  copy.strand = EditableStrand::make_default(app);
  copy.strand->logger().level = strand->logger().level;
  copy.client_connection_maker =
      [
        dsid_prefix = dsid_prefix, tcp_host = copy.tcp_host,
        tcp_port = copy.tcp_port
      ](LinkStrandRef & strand, const string_ &previous_session_id,
        int32_t last_ack_id) {
    return make_shared_<TcpClientConnection>(strand, dsid_prefix, tcp_host,
                                             tcp_port);
  };

  return std::move(copy);
}

ref_<DsLink> TestConfig::create_dslink(bool async) {
  if (tcp_server_port == 0) {
    throw "There is no server to connect right now. Please create a server first";
  }

  std::string address =
      std::string("ds://127.0.0.1:") + std::to_string(tcp_server_port);
  //        std::string("dss://127.0.0.1:") + std::to_string(tcp_secure_port);

  const char *argv[] = {"./test", "-b", address.c_str()};
  int argc = 3;
  auto link = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0", app);
  static_cast<ConsoleLogger &>(link->strand->logger()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;
  return link;
}

std::shared_ptr<TcpServer> TestConfig::create_server() {
  auto tcp_server = std::make_shared<TcpServer>(*this);
  tcp_server_port = tcp_server->get_port();
  tcp_secure_port = tcp_server->get_secure_port();
  return tcp_server;
}

void destroy_client_in_strand(ref_<Client> &client) {
  client->get_strand().post([client]() { client->destroy(); });
}

void destroy_dslink_in_strand(ref_<DsLink> &dslink) {
  dslink->strand->dispatch([dslink]() { dslink->destroy(); });
}
}

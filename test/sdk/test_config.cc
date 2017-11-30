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

#include "responder/model_base.h"
#include "responder/node_state_manager.h"

namespace dsa {

ref_<EditableStrand> TestConfig::make_editable_strand(
    const shared_ptr_<App> &app, bool async) {
  auto config =
      make_ref_<EditableStrand>(app->new_strand(), make_unique_<ECDH>());

  config->set_session_manager(make_ref_<SimpleSessionManager>(config));

  if (async) {
    config->set_security_manager(
        make_ref_<AsyncSimpleSecurityManager>(config->get_ref()));
  } else {
    config->set_security_manager(make_ref_<SimpleSecurityManager>());
  }

  config->set_logger(make_unique_<ConsoleLogger>());
  config->logger().level = Logger::WARN__;

  return config;
}

TestConfig::TestConfig(std::shared_ptr<App> app, bool async) : WrapperStrand() {
  this->app = app;
  strand = make_editable_strand(app, async);

  tcp_server_port = 0;
}

WrapperStrand TestConfig::get_client_wrapper_strand(bool async) {
  if (tcp_server_port == 0) {
    throw "There is no server to connect right now. Please create a server first";
  }

  WrapperStrand copy(*this);

  copy.tcp_server_port = 0;
  copy.tcp_host = "127.0.0.1";
  copy.tcp_port = tcp_server_port;

  copy.strand = make_editable_strand(app, async);
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
      std::string("127.0.0.1:") + std::to_string(tcp_server_port);

  const char *argv[] = {"./test", "-b", address.c_str()};
  int argc = 3;
  auto link = make_ref_<DsLink>(argc, argv, "mydslink", "1.0.0", app);

  return link;
}

std::shared_ptr<TcpServer> TestConfig::create_server() {
  auto tcp_server = std::make_shared<TcpServer>(*this);
  tcp_server_port = tcp_server->get_port();
  return tcp_server;
}

void destroy_client_in_strand(ref_<Client> &client) {
  client->get_strand().post([client]() { client->destroy(); });
}

void destroy_dslink_in_strand(ref_<DsLink> &dslink) {
  dslink->strand->dispatch([dslink]() { dslink->destroy(); });
}
}
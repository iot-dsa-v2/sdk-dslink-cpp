#include "dsa_common.h"

#include "test_config.h"

#include "core/app.h"
#include "core/session_manager.h"
#include "crypto/ecdh.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security_manager.h"
#include "network/tcp/tcp_client_connection.h"
#include "responder/model_base.h"
#include "responder/node_state_manager.h"

namespace dsa {

uint16_t TestConfig::_port = 4120;

static LinkConfig *make_config(App &app, bool async) {
  auto *config = new LinkConfig(app.new_strand(), make_unique_<ECDH>());

  config->set_session_manager(make_unique_<SessionManager>(config));

  if (async) {
    config->set_security_manager(
        make_unique_<AsyncSimpleSecurityManager>(config->get_ref()));
  } else {
    config->set_security_manager(make_unique_<SimpleSecurityManager>());
  }

  config->set_logger(make_unique_<ConsoleLogger>());
  config->logger().level = Logger::WARN_;

  return config;
}

TestConfig::TestConfig(App &app, bool async) : WrapperConfig() {
  strand.reset(make_config(app, async));

  tcp_server_port = _port++;
}

WrapperConfig TestConfig::get_client_config(App &app, bool async) {
  WrapperConfig copy(*this);

  copy.tcp_server_port = 0;
  copy.tcp_host = "127.0.0.1";
  copy.tcp_port = tcp_server_port;

  copy.strand.reset(make_config(app, async));
  copy.strand->logger().level = strand->logger().level;
  copy.client_connection_maker =
      [ dsid_prefix = dsid_prefix, tcp_host = copy.tcp_host, tcp_port = copy.tcp_port ](
          LinkStrandRef & strand, const string_ &previous_session_id,
          int32_t last_ack_id) {
    return make_shared_<ClientConnection>(strand, dsid_prefix, tcp_host,
                                          tcp_port);
  };

  return std::move(copy);
}

LinkConfig *TestConfig::get_link_config() {
  return static_cast<LinkConfig *>(strand.get());
}
}
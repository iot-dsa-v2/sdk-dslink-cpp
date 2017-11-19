#include "dsa_common.h"

#include "test_config.h"

#include "util/app.h"
#include "core/client.h"
#include "core/session_manager.h"
#include "crypto/ecdh.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security_manager.h"
#include "network/tcp/tcp_client_connection.h"
#include "responder/model_base.h"
#include "responder/node_state_manager.h"

namespace dsa {

uint16_t TestConfig::_port = 4120;

static ref_<EditableStrand> make_config(App &app, bool async) {
  auto config = make_ref_<EditableStrand>(app.new_strand(), make_unique_<ECDH>());

  config->set_session_manager(make_ref_<SessionManager>(config));

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

TestConfig::TestConfig(App &app, bool async) : WrapperStrand() {
  strand = make_config(app, async);

  tcp_server_port = _port++;
}

WrapperStrand TestConfig::get_client_wrapper_strand(App &app, bool async) {
  WrapperStrand copy(*this);

  copy.tcp_server_port = 0;
  copy.tcp_host = "127.0.0.1";
  copy.tcp_port = tcp_server_port;

  copy.strand = make_config(app, async);
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

void destroy_client_in_strand(ref_<Client> &client) {
  client->get_strand().dispatch([client]() { client->destroy(); });
}
}
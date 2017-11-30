#include "dsa_common.h"

#include "broker_runner.h"
#include "config/broker_config.h"
#include "config/module_loader.h"
#include "network/tcp/tcp_client_connection.h"

namespace dsa {
shared_ptr_<DsBroker> create_broker() {
  const char* empty_argv[1];
  ref_<BrokerConfig> broker_config = make_ref_<BrokerConfig>(0, empty_argv);
  ModuleLoader modules(broker_config);
  return make_shared_<DsBroker>(std::move(broker_config), modules);
}
WrapperStrand get_client_wrapper_strand(const shared_ptr_<DsBroker>& broker,
                                        const string_& dsid_prefix) {
  shared_ptr_<App>& app = broker->get_app();

  WrapperStrand client_strand;
  client_strand.tcp_host = "127.0.0.1";
  client_strand.tcp_port = broker->get_config()->port().get_value().get_int();

  client_strand.strand = TestConfig::make_editable_strand(app);
  client_strand.client_connection_maker =
      [
        dsid_prefix = dsid_prefix, tcp_host = client_strand.tcp_host,
        tcp_port = client_strand.tcp_port
      ](LinkStrandRef & strand, const string_& previous_session_id,
        int32_t last_ack_id)
          ->shared_ptr_<Connection> {
    return make_shared_<TcpClientConnection>(strand, dsid_prefix, tcp_host,
                                             tcp_port);
  };
  return std::move(client_strand);
}
}

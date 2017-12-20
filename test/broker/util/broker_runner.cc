#include "dsa_common.h"

#include "broker_runner.h"

#include "config/broker_config.h"
#include "config/module_loader.h"
#include "module/default/console_logger.h"
#include "module/logger.h"
#include "network/tcp/tcp_client_connection.h"

namespace dsa {
ref_<DsBroker> create_broker(std::shared_ptr<App> app) {
  const char* empty_argv[1];
  ref_<BrokerConfig> broker_config = make_ref_<BrokerConfig>(0, empty_argv);
  ModuleLoader modules(broker_config);
  auto broker = make_ref_<DsBroker>(std::move(broker_config), modules, app);
  // filter log for unit test
  static_cast<ConsoleLogger&>(broker->strand->logger()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;
  return std::move(broker);
}

WrapperStrand get_client_wrapper_strand(const ref_<DsBroker>& broker,
                                        const string_& dsid_prefix) {
  shared_ptr_<App>& app = broker->get_app();

  WrapperStrand client_strand;
  client_strand.dsid_prefix = dsid_prefix;
  client_strand.tcp_host = "127.0.0.1";
  client_strand.tcp_port = broker->get_config()->port().get_value().get_int();
  client_strand.strand = EditableStrand::make_default(app);
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

#include "dsa_common.h"

#include "broker_runner.h"

#include "config/broker_config.h"
#include "module/authorizer/broker_authorizer.h"
#include "module/client/broker_client_manager.h"
#include "module/default/console_logger.h"
#include "module/default/simple_storage.h"
#include "module/module_broker_default.h"
#include "network/tcp/stcp_client_connection.h"
#include "network/tcp/tcp_client_connection.h"
#include "network/ws/ws_client_connection.h"

namespace dsa {
ref_<DsBroker> create_broker(std::shared_ptr<App> app) {
  const char* empty_argv[1] = {"broker"};
  ref_<BrokerConfig> broker_config = make_ref_<BrokerConfig>(1, empty_argv);
  broker_config->port().set_value(Var(0));

  auto broker = make_ref_<DsBroker>(std::move(broker_config),
                                    make_ref_<ModuleBrokerDefault>(), app);
  // filter log for unit test
  static_cast<ConsoleLogger&>(Logger::_()).filter =
      Logger::FATAL_ | Logger::ERROR_ | Logger::WARN__;
  return std::move(broker);
}

WrapperStrand get_client_wrapper_strand(const ref_<DsBroker>& broker,
                                        const string_& dsid_prefix,
                                        dsa::ProtocolType protocol) {
  shared_ptr_<App>& app = broker->get_app();

  WrapperStrand client_strand;
  client_strand.dsid_prefix = dsid_prefix;
  client_strand.tcp_host = "127.0.0.1";

  client_strand.strand = EditableStrand::make_default(app);

  switch (protocol) {
    case dsa::ProtocolType::PROT_DSS:
      client_strand.tcp_port =
          broker->get_config()->secure_port().get_value().get_int();
      if (!client_strand.tcp_port) {
        client_strand.tcp_port = broker->get_active_secure_port();
      }
      client_strand.secure = true;
      break;
    case dsa::ProtocolType::PROT_WS:
      client_strand.ws_host = "127.0.0.1";
      // TODO: ws_port and ws_path
      client_strand.ws_port = 8080;
      client_strand.ws_path = "/";
      break;
    case dsa::ProtocolType::PROT_WSS:
      client_strand.ws_host = "127.0.0.1";
      // TODO: ws_port and ws_path
      client_strand.ws_port = 8443;
      client_strand.ws_path = "/";
      client_strand.secure = true;
      break;
    case dsa::ProtocolType::PROT_DS:
    default:
      if (broker->get_config()->port().get_value().get_int() != 0)
        client_strand.tcp_port =
            broker->get_config()->port().get_value().get_int();
      else
        client_strand.tcp_port = broker->get_active_server_port();
  }
  client_strand.set_client_connection_maker();

  return std::move(client_strand);
}
}

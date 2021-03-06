
#include "broker_dslink_util.h"

#include "module/authorizer/broker_authorizer.h"
#include "module/client/broker_client_manager.h"
#include "module/default/console_logger.h"
#include "module/default/simple_storage.h"

#include <module/module_broker_default.h>

namespace broker_dslink_test {

MockNodeChild::MockNodeChild(const LinkStrandRef &strand)
    : NodeModel(strand) {
  update_property("$is", Var("test_class"));
  update_property("@unit", Var("test_unit"));
};

bool MockNodeRoot::need_list() { return _need_list; }

MockNodeRoot::MockNodeRoot(const LinkStrandRef &strand)
    : NodeModel(strand) {
  add_list_child("child-a", make_ref_<MockNodeChild>(_strand));
  add_list_child("child-b", make_ref_<MockNodeChild>(_strand));
};

ref_<DsBroker> create_broker(std::shared_ptr<App> app) {
  const char *empty_argv[1] = {"broker"};
  ref_<BrokerConfig> broker_config = make_ref_<BrokerConfig>(1, empty_argv);
  broker_config->port().set_value(Var(0));

  auto broker = make_ref_<DsBroker>(std::move(broker_config),
                                    make_ref_<ModuleBrokerDefault>(), app);

  static_cast<ConsoleLogger &>(Logger::_()).filter =
      Logger::WARN__ | Logger::ERROR_ | Logger::FATAL_;

  return std::move(broker);
}

ref_<DsLink> create_dslink(std::shared_ptr<App> app, int port,
                           string_ dslink_name, bool connect,
                           dsa::ProtocolType protocol) {
  std::string address;

  switch (protocol) {
    case dsa::ProtocolType::PROT_DSS:
      address.assign(std::string("dss://127.0.0.1:") + std::to_string(port));
      break;
    case dsa::ProtocolType::PROT_WS:
      // TODO address.assign(std::string("ws://127.0.0.1:") +
      // std::to_string(ws_port));
      address.assign(std::string("ws://127.0.0.1:") + std::to_string(8080));
      break;
    case dsa::ProtocolType::PROT_WSS:
      // TODO address.assign(std::string("wss://127.0.0.1:") + std::to_string(ws_port));
      address.assign(std::string("wss://127.0.0.1:") + std::to_string(8443));
      break;
    case dsa::ProtocolType::PROT_DS:
    default:
      address.assign(std::string("ds://127.0.0.1:") + std::to_string(port));
  }

  const char *argv[] = {"./test", "-b", address.c_str()};
  int argc = 3;
  auto link = make_ref_<DsLink>(argc, argv, dslink_name, "1.0.0", app);
  static_cast<ConsoleLogger &>(Logger::_()).filter =
      Logger::WARN__ | Logger::ERROR_ | Logger::FATAL_;
  link->init_responder();

  if (connect) {
    bool connected = false;
    link->connect([&](const shared_ptr_<Connection> connection, ref_<DsLinkRequester> link_req) {
      connected = true;
    });

    WAIT_EXPECT_TRUE(1000, [&]() -> bool { return connected; });
  }

  return link;
}

ref_<DsLink> create_mock_dslink(std::shared_ptr<App> app, int port,
                                string_ dslink_name, bool connect,
                                dsa::ProtocolType protocol) {
  std::string address;

  switch (protocol) {
    case dsa::ProtocolType::PROT_DSS:
      address.assign(std::string("dss://127.0.0.1:") + std::to_string(port));
      break;
    case dsa::ProtocolType::PROT_WS:
      // TODO address.assign(std::string("ws://127.0.0.1:") +
      // std::to_string(ws_port));
      address.assign(std::string("ws://127.0.0.1:") + std::to_string(8080));
      break;
    case dsa::ProtocolType::PROT_WSS:
      // TODO address.assign(std::string("wss://127.0.0.1:") + std::to_string(ws_port));
      address.assign(std::string("wss://127.0.0.1:") + std::to_string(8443));
      break;
    case dsa::ProtocolType::PROT_DS:
    default:
      address.assign(std::string("ds://127.0.0.1:") + std::to_string(port));
  }

  const char *argv[] = {"./test", "-b", address.c_str()};
  int argc = 3;
  auto link = make_ref_<DsLink>(argc, argv, dslink_name, "1.0.0", app);
  static_cast<ConsoleLogger &>(Logger::_()).filter =
      Logger::WARN__ | Logger::ERROR_ | Logger::FATAL_;
  link->init_responder<MockNodeRoot>();

  if (connect) {
    bool connected = false;
    link->connect([&](const shared_ptr_<Connection> connection, ref_<DsLinkRequester> link_req) {
      connected = true;
    });

    WAIT_EXPECT_TRUE(1000, [&]() -> bool { return connected; });
  }

  return link;
}
}

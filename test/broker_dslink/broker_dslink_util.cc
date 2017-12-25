#include "broker_dslink_util.h"


namespace broker_dslink_test{

MockNodeChild::MockNodeChild(LinkStrandRef strand) : NodeModel(std::move(strand)) {
    update_property("$is", Var("test_class"));
    update_property("@unit", Var("test_unit"));
};

bool MockNodeRoot::need_list() { return _need_list; }

MockNodeRoot::MockNodeRoot(LinkStrandRef strand) : NodeModel(std::move(strand)) {
  add_list_child("child_a", make_ref_<MockNodeChild>(_strand));
  add_list_child("child_b", make_ref_<MockNodeChild>(_strand));
};

ref_<DsBroker> create_broker(std::shared_ptr<App> app) {
  const char* empty_argv[1];
  ref_<BrokerConfig> broker_config = make_ref_<BrokerConfig>(0, empty_argv);
  ModuleLoader modules(broker_config);

  ref_<DsBroker> broker;
  if(app != nullptr)
    broker = make_ref_<DsBroker>(std::move(broker_config), modules, app);
  else
    broker = make_ref_<DsBroker>(std::move(broker_config), modules);

  static_cast<ConsoleLogger &>(broker->strand->logger()).filter =
      Logger::WARN__ | Logger::ERROR_ | Logger::FATAL_;

  return std::move(broker);
}


ref_<DsLink> create_dslink(std::shared_ptr<App> app, int port, string_ dslink_name) {
  std::string address =
      std::string("127.0.0.1:") + std::to_string(port);

  const char *argv[] = {"./test", "-b", address.c_str()};
  int argc = 3;
  auto link = make_ref_<DsLink>(argc, argv, dslink_name, "1.0.0", app);
  static_cast<ConsoleLogger &>(link->strand->logger()).filter =
      Logger::WARN__ | Logger::ERROR_ | Logger::FATAL_;
  link->init_responder();

  return link;
}

ref_<DsLink> create_mock_dslink(std::shared_ptr<App> app, int port, string_ dslink_name) {
  std::string address =
      std::string("127.0.0.1:") + std::to_string(port);

  const char *argv[] = {"./test", "-b", address.c_str()};
  int argc = 3;
  auto link = make_ref_<DsLink>(argc, argv, dslink_name, "1.0.0", app);
  static_cast<ConsoleLogger &>(link->strand->logger()).filter =
      Logger::WARN__ | Logger::ERROR_ | Logger::FATAL_;
  link->init_responder<MockNodeRoot>();

  return link;
}

}

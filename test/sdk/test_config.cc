#include "dsa_common.h"

#include "test_config.h"

#include "core/app.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security_manager.h"
#include "responder/node_model.h"

namespace dsa {

class TestModel : public NodeModel {
public :
  TestModel(LinkStrandRef strand):NodeModel(std::move(strand)) {}

};

uint16_t TestConfig::_port = 4120;

static LinkConfig *make_config(App &app, bool async) {
  auto *config = new LinkConfig(app.new_strand(), new ECDH());

  config->set_session_manager(make_unique_<SessionManager>(config));
  config->set_stream_acceptor(
      make_unique_<NodeStateManager>(make_ref_<TestModel>(config->get_ref())));
  if (async) {
    config->set_security_manager(
        make_unique_<AsyncSimpleSecurityManager>((*config)()));
  } else {
    config->set_security_manager(make_unique_<SimpleSecurityManager>());
  }

  config->set_logger(make_unique_<ConsoleLogger>());
  config->logger().level = Logger::WARN_;

  return config;
}

TestConfig::TestConfig(App &app, bool async) : WrapperConfig() {
  strand.reset(make_config(app, async));

  tcp_host = "127.0.0.1";
  tcp_port = _port++;
}

WrapperConfig TestConfig::get_client_config(App &app, bool async) {
  WrapperConfig copy(*this);
  copy.strand.reset(make_config(app, async));
  return std::move(copy);
}

LinkConfig *TestConfig::get_link_config() {
  return static_cast<LinkConfig *>(strand.get());
}
}
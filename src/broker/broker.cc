#include "dsa_common.h"

#include "broker.h"

#include "config/broker_config.h"
#include "config/module_loader.h"
#include "module/logger.h"
#include "util/app.h"
#include "module/security_manager.h"

namespace dsa {
DsBroker::DsBroker(ref_<BrokerConfig>&& config, ModuleLoader& modules)
    : _config(std::move(config)) {
  init(modules);
}
DsBroker::~DsBroker() {}

void DsBroker::init(ModuleLoader& modules) {
  // init app
  size_t thread = static_cast<size_t>(_config->thread().get_value().get_int());
  if (thread < 1 || thread > 16) {
    thread = 1;
  }
  _app.reset(new App(thread));

  server_host = _config->port().get_value().get_string();
  tcp_server_port =
      static_cast<uint16_t>(_config->port().get_value().get_int());
  tcp_secure_port =
      static_cast<uint16_t>(_config->secure_port().get_value().get_int());
  uint16_t http_port =
      static_cast<uint16_t>(_config->http_port().get_value().get_int());
  uint16_t https_port =
      static_cast<uint16_t>(_config->https_port().get_value().get_int());

  strand.reset(new EditableStrand(
      _app->new_strand(), std::unique_ptr<ECDH>(ECDH::from_file(".key"))));

  // init logger
  std::unique_ptr<Logger> logger = modules.new_logger(*_app, strand);
  logger->level = Logger::parse(_config->log_level().get_value().to_string());
  strand->set_logger(std::move(logger));

  // init security manager
  strand->set_security_manager(modules.new_security_manager(*_app, strand));
}
void DsBroker::run() {
  strand->dispatch([]() {

  });
  _app->wait();
  destroy();
}
}

#include "dsa_common.h"

#include "broker.h"

#include <module/module_with_loader.h>
#include <util/string.h>
#include "config/broker_config.h"
#include "module/authorizer/broker_authorizer.h"
#include "module/client/broker_client_manager.h"
#include "module/client_manager.h"
#include "module/logger.h"
#include "module/module_broker_default.h"
#include "network/tcp/tcp_server.h"
#include "network/ws/ws_callback.h"
#include "node/broker_root.h"
#include "remote_node/broker_session_manager.h"
#include "remote_node/remote_root_node.h"
#include "responder/node_state_manager.h"
#include "upstream/upstream_manager.h"
#include "util/app.h"
#include "util/string.h"
#include "util/temp_file.h"
#include "web_server/web_server.h"

namespace dsa {
DsBroker::DsBroker(ref_<BrokerConfig>&& config, ref_<Module>&& modules,
                   const shared_ptr_<App>& app)
    : _config(std::move(config)), _app(app) {
  dsid_prefix = _config->name().get_value().get_string();
  init(std::move(modules));
}
DsBroker::~DsBroker() {}

void DsBroker::init(ref_<Module>&& default_module) {
  if (_app == nullptr) {
    // init app
    size_t thread =
        static_cast<size_t>(_config->thread().get_value().get_int());
    if (thread < 1 || thread > 16) {
      thread = 1;
    }
    _app.reset(new App(thread));
    _own_app = true;
  }

  TempFile::init("dsa-broker");

  server_host = _config->host().get_value().get_string();
  tcp_server_port =
      static_cast<uint16_t>(_config->port().get_value().get_int());
  tcp_secure_port =
      static_cast<int32_t>(_config->secure_port().get_value().get_int());
  uint16_t http_port =
      static_cast<uint16_t>(_config->http_port().get_value().get_int());
  uint16_t https_port =
      static_cast<uint16_t>(_config->https_port().get_value().get_int());

  strand.reset(new EditableStrand(_app->new_strand(),
                                  std::unique_ptr<ECDH>(ECDH::from_storage(
                                      Storage::get_config_bucket(), ".key"))));
  Logger::_().level =
      Logger::parse(_config->log_level().get_value().to_string());

  strand->dispatch([
    this, keepref = get_ref(), default_module = std::move(default_module)
  ]() mutable {
    if (default_module == nullptr)
      default_module = make_ref_<ModuleBrokerDefault>();

    modules = make_ref_<ModuleWithLoader>(_config->get_exe_path() / "modules",
                                          std::move(default_module));
    modules->init_all(*_app, strand);

    // init logger
    if (modules->get_logger().get() != nullptr &&
        modules->get_logger().get() != &Logger::_()) {
      modules->get_logger()->level =
          Logger::parse(_config->log_level().get_value().to_string());
      Logger::set_default(modules->get_logger());
    }

    // init storage
    strand->set_storage(modules->get_storage());

    // init security manager
    strand->set_client_manager(modules->get_client_manager());

    auto authorizer = modules->get_authorizer();
    strand->set_authorizer(std::move(authorizer));

    _master_token = get_master_token_from_storage(Storage::get_config_bucket());

    auto broker_root = make_ref_<BrokerRoot>(strand, get_ref());
    // init responder
    strand->set_stream_acceptor(
        make_ref_<NodeStateManager>(*strand, broker_root->get_ref()));

    // init session manager
    strand->set_session_manager(make_ref_<BrokerSessionManager>(
        strand, static_cast<NodeStateManager&>(strand->stream_acceptor())));

    modules->add_module_node(broker_root->get_sys(), broker_root->get_pub());

    _upstream = make_ref_<UpstreamManager>(strand);
    _upstream->add_node(broker_root->get_sys(), broker_root->get_pub());
  });
}
void DsBroker::destroy_impl() {
  _upstream->destroy();
  modules->destroy();

  if (_tcp_server != nullptr) {
    _tcp_server->destroy();
    _tcp_server.reset();
  }
  if (_web_server != nullptr) {
    _web_server->destroy();
    _web_server.reset();
  }
  _config.reset();

  WrapperStrand::destroy_impl();
  if (_own_app) {
    _app->close();
  }
}
void DsBroker::run(bool wait) {
  // start tcp server

  if (tcp_server_port >= 0 && tcp_server_port <= 65535) {
    strand->dispatch([this]() {
      _tcp_server = make_shared_<TcpServer>(*this);
      _tcp_server->start();
      LOG_INFO(__FILENAME__, LOG << "DsBroker started");
    });
  }

  // start web server

  strand->dispatch([this]() {
    // start web_server
    _web_server = std::make_shared<WebServer>(*_app);
    uint16_t http_port =
        static_cast<uint16_t>(_config->http_port().get_value().get_int());
    _web_server->listen(http_port);
    uint16_t https_port =
        static_cast<uint16_t>(_config->https_port().get_value().get_int());
    _web_server->secure_listen(https_port);
    _web_server->start();
    WebServer::WsCallback* root_cb = new WebServer::WsCallback();

    *root_cb =
        [this](
            WebServer& _web_server, std::unique_ptr<Websocket> websocket,
            http::request<request_body_t, http::basic_fields<alloc_t>>&& req) {
          DsaWsCallback dsa_ws_callback(strand);
          return dsa_ws_callback(_web_server.io_service(), std::move(websocket),
                                 std::move(req));
        };

    _web_server->add_ws_handler("/", std::move(*root_cb));
  });

  if (_own_app && wait) {
    _app->wait();
    destroy();
  }
}
int32_t DsBroker::get_active_server_port() {
  if (_tcp_server.get() == nullptr)
    return 0;
  else
    return _tcp_server->get_port();
}
int32_t DsBroker::get_active_secure_port() {
  if (_tcp_server.get() == nullptr)
    return 0;
  else
    return _tcp_server->get_secure_port();
}
void DsBroker::wait() {
  _app->wait();
  destroy();
}
}  // namespace dsa

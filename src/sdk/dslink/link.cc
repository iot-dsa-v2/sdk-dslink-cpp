#include "dsa_common.h"

#include "link.h"

#include <module/default/module_dslink_default.h>
#include <module/module_with_loader.h>
#include <util/string.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <fstream>

#include "core/client.h"
#include "crypto/ecdh.h"
#include "message/response/invoke_response_message.h"
#include "module/default/console_logger.h"
#include "module/default/dummy_stream_acceptor.h"
#include "module/default/simple_security.h"
#include "module/default/simple_session_manager.h"
#include "network/tcp/stcp_client_connection.h"
#include "network/tcp/tcp_client_connection.h"
#include "network/tcp/tcp_server.h"
#include "network/ws/ws_client_connection.h"
#include "node/link_root.h"
#include "responder/profile/pub_root.h"
#include "stream/requester/incoming_invoke_stream.h"
#include "stream/requester/incoming_set_stream.h"
#include "util/app.h"
#include "util/certificate.h"
#include "util/temp_file.h"

namespace opts = boost::program_options;
namespace fs = boost::filesystem;

namespace dsa {

DsLink::DsLink(int argc, const char *argv[], const string_ &link_name,
               const string_ &version, const shared_ptr_<App> &app,
               ref_<Module> &&default_module,
               bool use_standard_node_structure) {
  opts::options_description desc{"Options"};
  desc.add_options()("help,h", "Help screen")  //
      ("broker,b", opts::value<string_>()->default_value("127.0.0.1"),
       "Broker Url")  // broker url
      ("log,l", opts::value<string_>()->default_value("info"),
       "Log Level [all,trace,debug,info,warn,error,fatal,none]")  // log level
      ("token,t", opts::value<string_>()->default_value(""),
       "Token file path")  // token
      ("thread", opts::value<size_t>()->default_value(1),
       "Number of thread")  // custom name
      ("name,n", opts::value<string_>()->default_value(link_name),
       "Override Link Name")  // custom name
      ("server-port", opts::value<uint16_t>()->default_value(0),
       "Tcp Server Port")  // custom name
      ("module_path", opts::value<string_>()->default_value("./modules"),
       "Module Path")  // custom name
      ;

  try {
    _exe_path = boost::filesystem::canonical(
        boost::filesystem::system_complete(argv[0]).parent_path());
  } catch (const boost::filesystem::filesystem_error &ex) {
    LOG_FATAL(__FILENAME__, "link executable path is wrong!");
  }
  opts::variables_map variables;
  try {
    opts::store(opts::parse_command_line(argc, argv, desc), variables);
    opts::notify(variables);
  } catch (std::exception &e) {
    LOG_FATAL(__FILENAME__, LOG << "Invalid input, please check available "
                                   "parameters with --help\n");
  }

  // show help and exit
  if (variables.count("help")) {
    std::cout << desc << '\n';
    exit(0);
  }

  _app = app;
  // If app object is already given, thread option is ignored in args
  if (_app.get() == nullptr) {
    parse_thread(variables["thread"].as<size_t>());
    own_app = true;
  }
  TempFile::init("dslink-" + link_name);

  strand.reset(new EditableStrand(get_app().new_strand(),
                                  std::unique_ptr<ECDH>(ECDH::from_storage(
                                      Storage::get_config_bucket(), ".key"))));

  // TOKEN from file
  client_token = variables["token"].as<string_>();
  if (client_token.empty()) {
    if (Storage::get_config_bucket().exists("client-token")) {
      client_token =
          string_from_storage("client-token", Storage::get_config_bucket());
      if (client_token.empty()) {
        LOG_FATAL(__FILENAME__, LOG << "Failed to load client-token file");
      }
    }
  }

  // Master token from file
  master_token = get_master_token_from_storage(Storage::get_config_bucket());

  if (!parse_url(variables["broker"].as<string_>())) {
    LOG_FATAL(__FILENAME__, LOG << "Invalid Broker Url: "
                                << variables["broker"].as<string_>());
  }
  parse_name(variables["name"].as<string_>());

  // Adapted from parse_logger
  // Until we get module version we are using default one
  auto log = variables["log"].as<string_>();
  log_level_from_settings = Logger::parse(log);

  parse_server_port(variables["server-port"].as<uint16_t>());

  init_module(std::move(default_module), variables["module_path"].as<string_>(),
              use_standard_node_structure);

  LOG_TRACE(__FILENAME__, LOG << "DSLink initialized successfully");
}
void DsLink::init_module(ref_<Module> &&default_module,
                         const string_ &module_path,
                         bool use_standard_node_structure) {
  if (default_module == nullptr)
    default_module = make_ref_<ModuleDslinkDefault>();

  string_ final_module_path;
  if (module_path.empty() || !fs::is_directory(module_path)) {
    final_module_path = (_exe_path / "modules").string();
  } else {
    final_module_path = module_path;
  }

  modules =
      make_ref_<ModuleWithLoader>(final_module_path, std::move(default_module));
  modules->init_all(*_app, strand);

  strand->set_client_manager(modules->get_client_manager());
  strand->set_authorizer(modules->get_authorizer());

  modules->get_logger()->level = log_level_from_settings;
  Logger::set_default(modules->get_logger());

  strand->set_session_manager(make_ref_<SimpleSessionManager>(strand));
  if (use_standard_node_structure) {
    _root = make_ref_<LinkRoot>(strand, *this);
    strand->set_responder_model(_root->get_ref());
  }
}

DsLink::~DsLink() {}

App &DsLink::get_app() { return *_app; }

void DsLink::destroy_impl() {
  //  shared_ptr_<App> _app;
  //  shared_ptr_<TcpServer> _tcp_server;
  //  ref_<Client> _client;
  //
  //  bool _running = false;
  //
  //  OnConnectCallback _user_on_connect;
  modules->destroy();

  _root.reset();

  if (_tcp_server != nullptr) {
    _tcp_server->destroy();
    _tcp_server.reset();
  }
  if (_client) {
    _client->destroy();
    _client.reset();
  }

  // child remove itself from array
  while (!_subscribe_mergers.empty()) {
    // If you dont create lvalue from it
    // gets heap usage after free error because
    // reference count drops zero in destroy
    auto p = _subscribe_mergers.begin()->second;
    p->destroy();
  }

  // child remove itself from array
  while (!_list_mergers.empty()) {
    // If you dont create lvalue from it
    // gets heap usage after free error because
    // reference count drops zero in destroy
    auto p = _list_mergers.begin()->second;
    p->destroy();
  }

  WrapperStrand::destroy_impl();
  if (own_app) {
    _app->close();
  }
}

void DsLink::parse_thread(size_t thread) {
  if (thread < 1) {
    thread = 1;
  } else if (thread > 16) {
    thread = 16;
  }
  if (thread > 1) {
    // decode values in reading thread to improve multi-thread performance
    Message::decode_all = true;
  }
  _app.reset(new App(thread));
}

void DsLink::parse_name(const string_ &name) { dsid_prefix = name; }
void DsLink::parse_server_port(uint16_t port) { tcp_server_port = port; }

void DsLink::init_responder_raw(ref_<NodeModelBase> &&root_node) {
  if (_root != nullptr) {
    LOG_FATAL(__FILENAME__,
              LOG << "init_responder_raw called but root node is initialized");
  }
  strand->set_responder_model(std::move(root_node));
}
void DsLink::init_responder(ref_<NodeModelBase> &&main_node) {
  if (_root == nullptr) {
    LOG_FATAL(__FILENAME__, LOG << "init_responder called without root node");
  }
  if (main_node != nullptr) {
    _root->set_main(std::move(main_node));
  }
}

ref_<NodeModelBase> DsLink::add_to_main_node(const string_ &name,
                                             ref_<NodeModelBase> &&node) {
  return std::move(_root->add_to_main(name, std::move(node)));
}
void DsLink::remove_from_main_node(const string_ &name) {
  _root->remove_from_main(name);
}
ref_<NodeModel> DsLink::add_to_pub(const string_ &path,
                                   ref_<NodeModel> &&node) {
  return std::move(_root->get_pub_node()->add(path, std::move(node)));
}

void DsLink::connect(DsLink::LinkOnConnectCallback &&on_connect,
                     uint8_t callback_type) {
  if (_connected) {
    LOG_FATAL(__FILENAME__, LOG << "DsLink::connect(), Dslink is already "
                                   "requested for connection");
    return;
  }
  _connected = true;

  strand->dispatch([ =, on_connect = std::move(on_connect) ]() mutable {

    if (tcp_server_port > 0) {
      _tcp_server = make_shared_<TcpServer>(*this);
      _tcp_server->start();
    }

    set_client_connection_maker();

    _client = make_ref_<Client>(*this);
    _client->connect([ this, on_connect = std::move(on_connect) ](
                         const shared_ptr_<Connection> connection) {
      if (on_connect != nullptr) on_connect(connection, this->get_ref());
    },
                     callback_type);
  });
}

void DsLink::run(DsLink::LinkOnConnectCallback &&on_connect,
                 uint8_t callback_type) {
  if (_running) {
    LOG_FATAL(__FILENAME__, LOG << "DsLink::run(), Dslink is already running");
    return;
  }
  _running = true;

  if (!strand->is_responder_set()) {
    LOG_WARN(__FILENAME__, LOG << "responder is not initialized");
    _client->get_session().responder_enabled = false;
    strand->set_stream_acceptor(make_ref_<DummyStreamAcceptor>());
  }

  if (!_connected) {
    connect(std::move(on_connect), callback_type);
  } else {
    LOG_INFO(__FILENAME__, LOG << "DsLink on_connect callback "
                                  "ignored since it was connected "
                                  "before\n");
  }
  LOG_INFO(__FILENAME__, LOG << "DsLink running");
  if (own_app) {
    _app->wait();
    destroy();
  }
}

// requester features

ref_<IncomingSubscribeCache> DsLink::subscribe(
    const string_ &path, IncomingSubscribeCache::Callback &&callback,
    const SubscribeOptions &options) {
  if (_subscribe_mergers.count(path) == 0) {
    _subscribe_mergers[path] = make_ref_<SubscribeMerger>(get_ref(), path);
  }
  auto merger = _subscribe_mergers[path];
  return merger->subscribe(std::move(callback), options);
}

ref_<IncomingListCache> DsLink::list(const string_ &path,
                                     IncomingListCache::Callback &&callback,
                                     bool list_profile) {
  if (_list_mergers.count(path) == 0) {
    _list_mergers[path] = make_ref_<ListMerger>(get_ref(), path);
  }
  auto merger = _list_mergers[path];
  return merger->list(std::move(callback), list_profile);
}

ref_<IncomingInvokeStream> DsLink::invoke(
    IncomingInvokeStreamCallback &&callback,
    ref_<const InvokeRequestMessage> &&message) {
  return _client->get_session().invoke(
      CAST_LAMBDA(IncomingInvokeStreamCallback)([
        user_callback = std::move(callback),
        paged_cache = ref_<IncomingPageCache<InvokeResponseMessage>>()
      ](IncomingInvokeStream & s,
        ref_<const InvokeResponseMessage> && message) mutable {
        message = IncomingPageCache<InvokeResponseMessage>::get_first_page(
            paged_cache, std::move(message));
        if (message == nullptr) {
          // paged message is not ready
          return;
        }
        if (user_callback != nullptr) {
          user_callback(s, std::move(message));
        }
      }),
      std::move(message));
}

ref_<IncomingSetStream> DsLink::set(IncomingSetStreamCallback &&callback,
                                    ref_<const SetRequestMessage> &&message) {
  return _client->get_session().set(std::move(callback),
                                              std::move(message));
}
string_ DsLink::get_master_token() { return master_token; }
}  // namespace dsa

#include "dsa_common.h"

#include "link.h"

#include <util/string.h>
#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <regex>

#include "core/client.h"
#include "crypto/ecdh.h"
#include "module/default/console_logger.h"
#include "module/default/dummy_stream_acceptor.h"
#include "module/default/simple_security_manager.h"
#include "module/default/simple_session_manager.h"
#include "network/tcp/stcp_client_connection.h"
#include "network/tcp/tcp_client_connection.h"
#include "network/tcp/tcp_server.h"
#include "network/ws/ws_client_connection.h"
#include "node/link_root.h"
#include "stream/requester/incoming_invoke_stream.h"
#include "stream/requester/incoming_set_stream.h"
#include "util/app.h"
#include "util/string.h"

namespace opts = boost::program_options;
namespace fs = boost::filesystem;

namespace dsa {

DsLink::DsLink(int argc, const char *argv[], const string_ &link_name,
               const string_ &version, const shared_ptr_<App> &app) {
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
      ;

  opts::variables_map variables;
  try {
    opts::store(opts::parse_command_line(argc, argv, desc), variables);
    opts::notify(variables);
  } catch (std::exception &e) {
    LOG_FATAL(
        LOG
        << "Invalid input, please check available parameters with --help\n");
  }

  // show help and exit
  if (variables.count("help")) {
    std::cout << desc << '\n';
    exit(0);
  }

  own_app = false;
  _app = app;
  // If app object is already given, thread option is ignored in args
  if (_app.get() == nullptr) {
    parse_thread(variables["thread"].as<size_t>());
    own_app = true;
  }

  strand.reset(new EditableStrand(
      get_app().new_strand(), std::unique_ptr<ECDH>(ECDH::from_file(".key"))));

  // TOKEN from file
  client_token = "";
  auto client_token_path = variables["token"].as<string_>();
  if (client_token_path.length() != 0) {
    try {
      client_token = string_from_file(client_token_path);
    } catch (std::exception &e) {
      LOG_FATAL(LOG << "Fatal loading token file " << client_token_path
                    << " with error : " << e.what());
    }
  }

  close_token = get_close_token_from_file();

  parse_url(variables["broker"].as<string_>());
  parse_name(variables["name"].as<string_>());
  parse_log(variables["log"].as<string_>(), *strand);
  parse_server_port(variables["server-port"].as<uint16_t>());

  LOG_TRACE(strand.get()->logger(), LOG << "DSLink initialized successfully");
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
  _app.reset(new App(thread));
}
void DsLink::parse_url(const string_ &url) {
  static std::regex url_regex(
      R"(^(ds://|dss://|ws://|wss://)?([^/:\[]+|\[[0-9A-Fa-f:]+\])(:\d+)?(/.*)?$)");

  auto match = std::sregex_iterator(url.begin(), url.end(), url_regex);
  if (match == std::sregex_iterator()) {  // match is a empty iterator
    LOG_FATAL(LOG << "Invalid Broker Url: " << url);
  }
  string_ protocol = (*match)[1].str();

  if (protocol == "ws://" || protocol == "wss://") {
    if (protocol == "wss://") {
      secure = true;
      ws_port = 443;  // default wss port
    } else {
      ws_port = 80;  // default ws port
    }
    ws_host = (*match)[2].str();
    if ((*match)[3].length() > 1) {
      ws_port = static_cast<uint16_t>(
          std::stoi(string_((*match)[3].first + 1, (*match)[3].second)));
    }
    ws_path = (*match)[4].str();
  } else {
    if (protocol == "dss://") {
      secure = true;
      tcp_port = 4128;  // default dss port
    } else {            // "ds://" or blank
      tcp_port = 4120;  // default ds port
    }
    tcp_host = (*match)[2].str();
    if ((*match)[3].length() > 1) {
      tcp_port = static_cast<uint16_t>(
          std::stoi(string_((*match)[3].first + 1, (*match)[3].second)));
    }
  }
}

void DsLink::parse_log(const string_ &log, EditableStrand &config) {
  auto logger = std::unique_ptr<Logger>(new ConsoleLogger());
  logger->level = Logger::parse(log);
  config.set_logger(std::move(logger));
}
void DsLink::parse_name(const string_ &name) { dsid_prefix = name; }
void DsLink::parse_server_port(uint16_t port) { tcp_server_port = port; }

void DsLink::init_responder_raw(ref_<NodeModelBase> &&root_node) {
  strand->set_session_manager(make_ref_<SimpleSessionManager>(strand));
  strand->set_security_manager(make_ref_<SimpleSecurityManager>());
  strand->set_responder_model(std::move(root_node));
}
void DsLink::init_responder(ref_<NodeModelBase> &&main_node) {
  _root = make_ref_<LinkRoot>(strand->get_ref(), get_ref());
  if (main_node != nullptr) {
    _root->set_main(std::move(main_node));
  }
  init_responder_raw(_root->get_ref());
}

ref_<NodeModelBase> DsLink::add_to_main_node(const string_ &name,
                                             ref_<NodeModel> &&node) {
  return std::move(_root->add_to_main(name, std::move(node)));
}
void DsLink::remove_from_main_node(const string_ &name) {
  _root->remove_from_main(name);
}

void DsLink::connect(Client::OnConnectCallback &&on_connect,
                     uint8_t callback_type) {
  if (_connected) {
    LOG_FATAL(
        LOG << "DsLink::connect(), Dslink is already requested for connection");
    return;
  }
  _connected = true;

  strand->dispatch([ =, on_connect = std::move(on_connect) ]() mutable {

    if (tcp_server_port > 0) {
      _tcp_server = make_shared_<TcpServer>(*this);
      _tcp_server->start();
    }

    if (tcp_port > 0) {
      if (secure) {
        static boost::asio::ssl::context context(
            boost::asio::ssl::context::sslv23);
        boost::system::error_code error;
        context.load_verify_file("certificate.pem", error);
        if (error) {
          LOG_FATAL(LOG << "Failed to verify cetificate");
        }

        client_connection_maker =
            [
              &context, dsid_prefix = dsid_prefix, tcp_host = tcp_host,
              tcp_port = tcp_port
            ](LinkStrandRef & strand, const string_ &previous_session_id,
              int32_t last_ack_id) {
          return make_shared_<StcpClientConnection>(
              strand, context, dsid_prefix, tcp_host, tcp_port);
        };
      } else {
        client_connection_maker =
            [
              dsid_prefix = dsid_prefix, tcp_host = tcp_host,
              tcp_port = tcp_port
            ](LinkStrandRef & strand, const string_ &previous_session_id,
              int32_t last_ack_id) {
          return make_shared_<TcpClientConnection>(strand, dsid_prefix,
                                                   tcp_host, tcp_port);
        };
      }
    } else if (ws_port > 0) {
      client_connection_maker =
          [ dsid_prefix = dsid_prefix, ws_host = ws_host, ws_port = ws_port ](
              LinkStrandRef & strand, const string_ &previous_session_id,
              int32_t last_ack_id) {
        return make_shared_<WsClientConnection>(strand, dsid_prefix, ws_host,
                                                ws_port);
      };
    }

    _client = make_ref_<Client>(*this);
    _client->connect(std::move(on_connect), callback_type);
    LOG_SYSTEM(strand.get()->logger(), LOG << "DsLink connection requested");
  });
}

void DsLink::run(Client::OnConnectCallback &&on_connect,
                 uint8_t callback_type) {
  if (_running) {
    LOG_FATAL(LOG << "DsLink::run(), Dslink is already running");
    return;
  }
  _running = true;

  if (!strand->is_responder_set()) {
    LOG_WARN(strand->logger(), LOG << "responder is not initialized");
    _client->get_session().responder_enabled = false;
    strand->set_stream_acceptor(make_ref_<DummyStreamAcceptor>());
  }

  if (!_connected) {
    connect(std::move(on_connect), callback_type);
  } else {
    LOG_SYSTEM(strand.get()->logger(),
               LOG << "DsLink on_connect callback "
                      "ignored since it was connected "
                      "before\n");
  }
  LOG_SYSTEM(strand.get()->logger(), LOG << "DsLink running");
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
                                     IncomingListCache::Callback &&callback) {
  if (_list_mergers.count(path) == 0) {
    _list_mergers[path] = make_ref_<ListMerger>(get_ref(), path);
  }
  auto merger = _list_mergers[path];
  return merger->list(std::move(callback));
}
ref_<IncomingInvokeStream> DsLink::invoke(
    IncomingInvokeStreamCallback &&callback,
    ref_<const InvokeRequestMessage> &&message) {
  return _client->get_session().requester.invoke(std::move(callback),
                                                 std::move(message));
}

ref_<IncomingSetStream> DsLink::set(IncomingSetStreamCallback &&callback,
                                    ref_<const SetRequestMessage> &&message) {
  return _client->get_session().requester.set(std::move(callback),
                                              std::move(message));
}
string_ DsLink::get_close_token() { return close_token; }
}

#include "dsa_common.h"

#include "link.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <regex>

#include "core/client.h"
#include "core/session_manager.h"
#include "crypto/ecdh.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security_manager.h"
#include "network/tcp/tcp_client_connection.h"
#include "network/tcp/tcp_server.h"
#include "stream/requester/incoming_invoke_stream.h"
#include "stream/requester/incoming_set_stream.h"
#include "util/app.h"

namespace opts = boost::program_options;
namespace fs = boost::filesystem;

namespace dsa {

DsLink::DsLink(int argc, const char *argv[], const string_ &link_name,
               const string_ &version) {
  opts::options_description desc{"Options"};
  desc.add_options()("help,h", "Help screen")  //
      ("broker,b", opts::value<string_>()->default_value("127.0.0.1"),
       "Broker Url")  // broker url
      ("log,l", opts::value<string_>()->default_value("info"),
       "Log Level [all,trace,debug,info,warn,error,fatal,none]")  // log level
      ("thread,t", opts::value<size_t>()->default_value(1),
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
    std::cout
        << "Invalid input, please check available parameters with --help\n";
    exit(1);
  }

  // show help and exit
  if (variables.count("help")) {
    std::cout << desc << '\n';
    exit(0);
  }

  parse_thread(variables["thread"].as<size_t>());

  auto *config = new LinkConfig(get_app().new_strand(), load_private_key());
  strand.reset(config);

  parse_url(variables["broker"].as<string_>());
  parse_name(variables["name"].as<string_>());
  parse_log(variables["log"].as<string_>(), *config);
  parse_server_port(variables["server-port"].as<uint16_t>());
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
  _app->close();

  WrapperConfig::destroy_impl();
}
std::unique_ptr<ECDH> DsLink::load_private_key() {
  fs::path path(".key");

  try {
    if (fs::is_regular_file(path) && fs::file_size(path) == 32) {
      std::ifstream keyfile(".key", std::ios::in | std::ios::binary);
      if (keyfile.is_open()) {
        uint8_t data[32];
        keyfile.read(reinterpret_cast<char *>(data), 32);
        return make_unique_<ECDH>(data, 32);

      } else {
        LOG_FATAL(LOG << "Unable to open .key file");
        // file exists but can't open, make a new kwy won't solve the problem
      }
    }
  } catch (std::exception &e) {
    LOG_ERROR(Logger::_(),
              LOG << "error loading existing private key, generating new key");
  }

  auto newkey = make_unique_<ECDH>();

  std::ofstream keyfile(".key",
                        std::ios::out | std::ios::binary | std::ios::trunc);
  if (keyfile.is_open()) {
    auto data = newkey->get_private_key();
    keyfile.write(reinterpret_cast<char *>(data.data()), data.size());
  } else {
    LOG_FATAL(LOG << "Unable to open .key file");
  }
  return std::move(newkey);
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

void DsLink::parse_log(const string_ &log, LinkConfig &config) {
  auto *logger = new ConsoleLogger();
  config.set_logger(std::unique_ptr<Logger>(logger));
  logger->level = Logger::parse(log);
}
void DsLink::parse_name(const string_ &name) { dsid_prefix = name; }
void DsLink::parse_server_port(uint16_t port) { tcp_server_port = port; }

void DsLink::init_responder(ref_<NodeModelBase> &&root_node) {
  strand->set_session_manager(make_ref_<SessionManager>(strand));
  strand->set_security_manager(make_ref_<SimpleSecurityManager>());

  strand->set_responder_model(std::move(root_node));
}

void DsLink::run(OnConnectCallback &&on_connect, uint8_t callback_type) {
  if (_running) {
    LOG_FATAL(LOG << "DsLink::run(), Dslink is already running");
  }
  _running = true;

  strand->dispatch([ =, on_connect = std::move(on_connect) ]() mutable {

    if (tcp_server_port > 0) {
      _tcp_server = make_shared_<TcpServer>(*this);
      _tcp_server->start();
    }

    if (tcp_port > 0) {
      if (secure) {
        // TODO implement secure client
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
      // TODO, implement ws client
    }

    _client = make_ref_<Client>(*this);
    _client->connect(std::move(on_connect), callback_type);
  });
  _app->wait();
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
    const string_ &path, IncomingInvokeStreamCallback &&callback,
    ref_<const InvokeRequestMessage> &&message) {
  return _client->get_session().requester.invoke(path, std::move(callback),
                                                 std::move(message));
}

ref_<IncomingSetStream> DsLink::set(const string_ &path,
                                    IncomingSetStreamCallback &&callback,
                                    ref_<const SetRequestMessage> &&message) {
  return _client->get_session().requester.set(path, std::move(callback),
                                              std::move(message));
}
}

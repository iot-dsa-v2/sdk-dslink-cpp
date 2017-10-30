#include "dsa_common.h"

#include "config_loader.h"

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <iostream>
#include <regex>

#include "core/app.h"
#include "crypto/ecdh.h"
#include "module/default/console_logger.h"

namespace opts = boost::program_options;
namespace fs = boost::filesystem;

namespace dsa {

ConfigLoader::ConfigLoader(int argc, const char *argv[],
                           const string_ &link_name, const string_ &version) {
  opts::options_description desc{"Options"};
  desc.add_options()("help,h", "Help screen")  //
      ("broker,b", opts::value<string_>()->default_value("localhost"),
       "Broker Url")  // broker url
      ("log,l", opts::value<string_>()->default_value("info"),
       "Log Level [all,trace,debug,info,warn,error,fatal,none]")  // log level
      ("thread,t", opts::value<size_t>()->default_value(1),
       "Number of thread")  // custom name
      ("name,n", opts::value<string_>()->default_value(link_name),
       "Override Link Name")  // custom name
      ;

  opts::variables_map variables;
  opts::store(opts::parse_command_line(argc, argv, desc), variables);
  opts::notify(variables);

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
}

std::unique_ptr<ECDH> ConfigLoader::load_private_key() {
  fs::path path(".key");

  try {
    if (fs::is_regular_file(path) && fs::file_size(path) == 32) {
      std::ifstream keyfile(".key", std::ios::in | std::ios::binary);
      if (keyfile.is_open()) {
        uint8_t data[32];
        keyfile.read(reinterpret_cast<char *>(data), 32);
        return std::unique_ptr<ECDH>(new ECDH(data, 32));

      } else {
        std::cout << "Unable to open .key file";
        // file exists but can't open, make a new kwy won't solve the problem
        exit(1);
      }
    }
  } catch (std::exception &e) {
    std::cout << "error loading private key, generating new key";
  }

  auto newkey = std::unique_ptr<ECDH>(new ECDH());

  std::ofstream keyfile(".key",
                        std::ios::out | std::ios::binary | std::ios::trunc);
  if (keyfile.is_open()) {
    auto data = newkey->get_private_key();
    keyfile.write(reinterpret_cast<char *>(data.data()), data.size());
  } else {
    std::cout << "Unable to open .key file";
    exit(1);
  }
  return std::move(newkey);
}
void ConfigLoader::parse_thread(size_t thread) {
  if (thread < 1) {
    thread = 1;
  } else if (thread > 16) {
    thread = 16;
  }
  _app.reset(new App(thread));
}
void ConfigLoader::parse_url(const string_ &url) {
  static std::regex url_regex(
      R"(^(ds://|dss://|ws://|wss://)?([^/:\[]+|\[[0-9A-Fa-f:]+\])(:\d+)?(/.*)?$)");

  auto match = std::sregex_iterator(url.begin(), url.end(), url_regex);
  if (match == std::sregex_iterator()) {  // match is a empty iterator
    std::cout << std::endl << "Invalid Broker Url: " << url;
    exit(0);
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

void ConfigLoader::parse_log(const string_ &log, LinkConfig &config) {
  auto *logger = new ConsoleLogger();
  config.set_logger(std::unique_ptr<Logger>(logger));
  if (log == "all") {
    logger->level = Logger::ALL_;
  } else if (log == "trace") {
    logger->level = Logger::TRACE_;
  } else if (log == "debug") {
    logger->level = Logger::DEBUG_;
  } else if (log == "warn") {
    logger->level = Logger::WARN_;
  } else if (log == "error") {
    logger->level = Logger::ERROR_;
  } else if (log == "fatal") {
    logger->level = Logger::FATAL_;
  } else if (log == "none") {
    logger->level = Logger::NONE_;
  } else {  // default
    logger->level = Logger::INFO_;
  }
}
void ConfigLoader::parse_name(const string_ &name) { dsid_prefix = name; }
}

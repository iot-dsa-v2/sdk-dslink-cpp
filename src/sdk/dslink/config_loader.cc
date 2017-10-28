#include "dsa_common.h"

#include "config_loader.h"

#include <boost/program_options.hpp>
#include <iostream>
#include <regex>

#include "core/app.h"
#include "crypto/ecdh.h"

namespace opts = boost::program_options;

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

  LinkConfig *config =
      new LinkConfig(get_app().new_strand(), load_private_key());

  parse_url(variables["broker"].as<string_>());
  parse_name(variables["name"].as<string_>());
  parse_log(variables["name"].as<string_>());
}

std::unique_ptr<ECDH> ConfigLoader::load_private_key() {}
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

void ConfigLoader::parse_log(const string_ &log) {}
void ConfigLoader::parse_name(const string_ &name) {}
}

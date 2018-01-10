#include "dsa_common.h"

#include "broker_config.h"

#include <boost/filesystem.hpp>
#include <iostream>
#include "module/logger.h"

namespace fs = boost::filesystem;

namespace dsa {
BrokerConfig::BrokerConfig(int argc, const char* argv[]) {
  init();
  load();
}

const string_& BrokerConfig::get_file_path() {
  static string_ default_path = "broker.json";

  if (_file_path.empty()) {
    return default_path;
  }
  return _file_path;
}

// init all the config properties
void BrokerConfig::init() {
  add_item("thread", Var(2), VarValidatorInt(1, 100));
  add_item("host", Var("0.0.0.0"), [](const Var& var) {
    return var.is_string() && !var.get_string().empty();
  });
  add_item("port", Var(4120), VarValidatorInt(1, 65535));
  add_item("secure-port", Var(-1), VarValidatorInt(-1, 65535));
  add_item("http-port", Var(8080), VarValidatorInt(1, 65535));
  add_item("https-port", Var(8443), VarValidatorInt(1, 65535));
  add_item("log-level", Var("warn"), [](const Var& var) {
    string_ str = var.to_string();
    return str == "trace" || str == "debug" || str == "info" || str == "warn" ||
           str == "error" || str == "fatal";
  });
}
// load config json from file
void BrokerConfig::load() {
  auto& path = get_file_path();
  if (fs::exists(path)) {
    if (fs::is_regular_file(path)) {
      std::ifstream config_file(get_file_path(), std::ios::in);
      if (config_file.is_open()) {
        std::stringstream buffer;
        buffer << config_file.rdbuf();
        Var data = Var::from_json(buffer.str());
        if (data.is_map()) {
          try {
            // allows config to be stored in different location
            if (_file_path.empty() && data.get_map().count("config-path") > 0 &&
                !data["config-path"].to_string().empty()) {
              // load broker config from different path
              _file_path = data["config-path"].get_string();
              load();
              return;
            }
          } catch (std::exception& e) {
            // config-path doesn't exist, use default
          }
          for (auto& it : data.get_map()) {
            auto search = _items.find(it.first);
            if (search != _items.end()) {
              search->second.set_value(std::move(it.second));
            }
          }
          return;
        }
      }
    }

    LOG_FATAL(LOG << "failed to open broker config file: " << path);
  } else {
    // config doesn't exist, write a default config file
    save();
  }
}
void BrokerConfig::save() {
  auto& path = get_file_path();
  std::ofstream config_file(path, std::ios::out | std::ios::trunc);
  if (config_file.is_open()) {
    config_file << "{\n"
                << R"("dsa-version": ")" << int(DSA_MAJOR_VERSION) << "."
                << int(DSA_MINOR_VERSION) << "\",\n";
#ifdef DSA_DEBUG
    config_file << R"("broker-build": "debug")";
#else
    config_file << R"("broker-build": "release")";
#endif
    for (auto& name : _names) {
      config_file << ",\n\"" << name << "\": ";
      config_file << _items[name].get_value().to_json(2);
    }
    config_file << "\n}";
  } else {
    LOG_ERROR(Logger::_(), LOG << "failed to write the broker config file");
  }
}
void BrokerConfig::add_item(const string_& name, Var&& value,
                            VarValidator&& validator) {
  _names.emplace_back(name);
  _items.emplace(name,
                 BrokerConfigItem(std::move(value), std::move(validator)));
}
}

#include "dsa_common.h"

#include "broker_config.h"

#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
#include "module/logger.h"

namespace fs = boost::filesystem;

namespace dsa {
BrokerConfig::BrokerConfig(int argc, const char* argv[]) {
  init();
  storage_key = get_file_path();
  storage_bucket = simple_storage.get_bucket(storage_key);
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
  add_item("port", Var(4120), VarValidatorInt(0, 65535));
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
  auto read_callback = [=](std::string storage_key, std::vector<uint8_t> vec) {

    // const string_* content = reinterpret_cast<const string_*>(vec.data());
    std::string content(vec.begin(), vec.end());
    if (!content.empty()) {
      Var data = Var::from_json(content.c_str());
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
    } else {
      save();
    }
  };
  storage_bucket->read(storage_key, read_callback);

}
void BrokerConfig::save() {
  std::stringstream config_file;
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

  const char* cstr = config_file.str().c_str();

  auto data = new RefCountBytes(&cstr[0], &cstr[strlen(cstr)]);

  storage_bucket->write(storage_key, std::forward<RefCountBytes*>(data));
}
void BrokerConfig::add_item(const string_& name, Var&& value,
                            VarValidator&& validator) {
  _names.emplace_back(name);
  _items.emplace(name,
                 BrokerConfigItem(std::move(value), std::move(validator)));
}
}

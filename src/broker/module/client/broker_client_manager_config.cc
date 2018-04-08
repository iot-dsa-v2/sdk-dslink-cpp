#include "dsa_common.h"

#include "broker_client_manager_config.h"

#include <string>
#include "module/logger.h"
#include "module/storage.h"

namespace dsa {
const string_ config_bucket_key = "client-manager.json";
BrokerClientManagerConfig::BrokerClientManagerConfig() {
  init();
  load();
}
// init all the config properties
void BrokerClientManagerConfig::init() {
  add_item("enable-quarantine", Var(false),
           [](const Var& var) { return var.is_bool(); });
  add_item("allow-all-links", Var(true),
           [](const Var& var) { return var.is_bool(); });
}
// load config json from file
void BrokerClientManagerConfig::load() {
  auto read_callback = [=](string_ storage_key, std::vector<uint8_t> vec,
                           BucketReadStatus read_status) {

    // const string_* content = reinterpret_cast<const string_*>(vec.data());
    std::string content(vec.begin(), vec.end());
    if (!content.empty()) {
      Var data = Var::from_json(content.c_str());
      if (data.is_map()) {
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
  Storage::get_config_bucket().read(config_bucket_key, read_callback);
}
void BrokerClientManagerConfig::save() {
  std::stringstream config_file;
  config_file << "{\n";
  for (int i = 0; i < _names.size();) {
    config_file << "\"" << _names[i] << "\": ";
    config_file << _items[_names[i]].get_value().to_json(2);
    if (++i < _names.size()) {
      config_file << ",\n";
    }
  }
  config_file << "\n}";

  const std::string& tmp_str = config_file.str();

  const char* cstr = tmp_str.c_str();

  auto data = new RefCountBytes(&cstr[0], &cstr[strlen(cstr)]);

  Storage::get_config_bucket().write(config_bucket_key,
                                     std::forward<RefCountBytes*>(data));
}
void BrokerClientManagerConfig::add_item(const string_& name, Var&& value,
                                         VarValidator&& validator) {
  _names.emplace_back(name);
  _items.emplace(name,
                 BrokerConfigItem(std::move(value), std::move(validator)));
}
}

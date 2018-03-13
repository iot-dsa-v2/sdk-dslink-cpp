#ifndef DSA_SDK_BROKER_CLIENT_MANAGER_CONFIG_H
#define DSA_SDK_BROKER_CLIENT_MANAGER_CONFIG_H


#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>
#include <vector>

#include <boost/filesystem.hpp>
#include "../config/broker_config_item.h"
#include "module/default/simple_storage.h"
#include "util/enable_ref.h"

namespace dsa {

class BrokerClientManagerConfig : public EnableRef<BrokerClientManagerConfig> {

  // a map for all the config key values
  std::unordered_map<string_, BrokerConfigItem> _items;
  // keep an order of the items so they are saved in a nice order
  std::vector<string_> _names;

  // init all the config properties
  void init();
  // load config json from file
  void load();

  void add_item(const string_& name, Var&& value, VarValidator&&);

 public:
  BrokerClientManagerConfig();
  BrokerConfigItem& enable_quarantine() { return _items["enable-quarantine"]; }
  BrokerConfigItem& allow_all_links() { return _items["allow-all-links"]; }
  void save();

};

}  // namespace dsa

#endif //DSA_SDK_BROKER_CLIENT_MANAGER_CONFIG_H

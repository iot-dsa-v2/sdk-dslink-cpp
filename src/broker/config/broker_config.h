#ifndef DSA_BROKER_CONFIG_H
#define DSA_BROKER_CONFIG_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>
#include <vector>

#include "broker_config_item.h"
#include "util/enable_ref.h"

namespace dsa {
class BrokerConfig : public EnableRef<BrokerConfig> {
  string_ _file_path;
  const string_& get_file_path();
  // a map for all the config key values
  std::unordered_map<string_, BrokerConfigItem> _items;
  // keep an order of the items so they are saved in a nice order
  std::vector<string_> _names;

  // init all the config properties
  void init();
  // load config json from file
  void load();
  void save();

  void add_item(const string_& name, Var&& value, int type = -1);

 public:
  BrokerConfig(int argc, const char* argv[]);
};
}

#endif  // DSA_BROKER_CONFIG_H

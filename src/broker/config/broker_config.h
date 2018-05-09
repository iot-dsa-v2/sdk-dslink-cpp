#ifndef DSA_BROKER_CONFIG_H
#define DSA_BROKER_CONFIG_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>
#include <vector>

#include <boost/filesystem.hpp>
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
  boost::filesystem::path _exe_path;

  // init all the config properties
  void init();
  // load config json from file
  void load();
  void save();

  void add_item(const string_& name, Var&& value, VarValidator&&);

 public:
  BrokerConfig(int argc, const char* argv[]);
  BrokerConfigItem& name() { return _items["name"]; }
  BrokerConfigItem& thread() { return _items["thread"]; }
  BrokerConfigItem& host() { return _items["host"]; }
  BrokerConfigItem& port() { return _items["port"]; }
  BrokerConfigItem& secure_port() { return _items["secure-port"]; }
  BrokerConfigItem& http_port() { return _items["http-port"]; }
  BrokerConfigItem& https_port() { return _items["https-port"]; }
  BrokerConfigItem& log_level() { return _items["log-level"]; }

  const boost::filesystem::path& get_exe_path() { return _exe_path; };
};
}

#endif  // DSA_BROKER_CONFIG_H

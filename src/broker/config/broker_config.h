#ifndef DSA_BROKER_CONFIG_H
#define DSA_BROKER_CONFIG_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>

#include "broker_config_item.h"
#include "util/enable_ref.h"

namespace dsa {
class BrokerConfig : public EnableRef<BrokerConfig> {
  std::unordered_map<string_, BrokerConfigItem> _items;

  // init all the config properties
  void init();
  // load config json from file
  void load();

 public:
  BrokerConfig(int argc, const char *argv[]);
};
}

#endif  // DSA_BROKER_CONFIG_H

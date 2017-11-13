#ifndef DSA_BROKER_CONFIG_H
#define DSA_BROKER_CONFIG_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "util/enable_ref.h"

namespace dsa {
class BrokerConfig : public EnableRef<BrokerConfig> {
 public:
  BrokerConfig(int argc, const char *argv[]);
};
}

#endif  // DSA_BROKER_CONFIG_H

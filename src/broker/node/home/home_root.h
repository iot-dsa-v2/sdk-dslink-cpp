#ifndef DSA_BROKER_HOME_ROOT_H
#define DSA_BROKER_HOME_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {

class BrokerHomeRoot : public NodeModel {
 public:
  explicit BrokerHomeRoot(const LinkStrandRef &strand);

  bool allows_runtime_child_change() final { return true; }
};
}

#endif  // DSA_BROKER_HOME_ROOT_H

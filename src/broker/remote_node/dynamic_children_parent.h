#ifndef DSA_SDK_DYNAMIC_CHILDREN_PARENT_H
#define DSA_SDK_DYNAMIC_CHILDREN_PARENT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {

class DynamicChildrenParent : public NodeModel {
 public:
  explicit DynamicChildrenParent(const LinkStrandRef &strand);

  bool allows_runtime_child_change() final { return true; }
  ModelRef on_demand_create_child(const Path &path) final {
    return UNAVAILABLE;
  }
};
}


#endif //DSA_SDK_DYNAMIC_CHILDREN_PARENT_H

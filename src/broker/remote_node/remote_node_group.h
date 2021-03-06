#ifndef DSA_SDK_REMOTE_NODE_GROUP_H
#define DSA_SDK_REMOTE_NODE_GROUP_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "remote_root_node.h"
#include "responder/node_model.h"

namespace dsa {

class RemoteNodeGroup : public NodeModel {
 public:
  RemoteNodeGroup(const LinkStrandRef &strand);
  ~RemoteNodeGroup() override;

  virtual ref_<RemoteRootNode> create_remote_root(const string_ &name,
                                                  BaseSession &session);

  bool allows_runtime_child_change() final { return true; }
  ModelRef on_demand_create_child(const Path &path) final {
    return UNAVAILABLE;
  }
};
}  // namespace dsa

#endif  // DSA_SDK_REMOTE_NODE_GROUP_H

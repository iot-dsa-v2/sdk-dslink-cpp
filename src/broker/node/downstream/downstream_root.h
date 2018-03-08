#ifndef DSA_BROKER_DOWNSTREAM_ROOT_H
#define DSA_BROKER_DOWNSTREAM_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {
class RemoteRootNode;
class Session;
struct ClientInfo;

class DownstreamRoot : public NodeModel {
 public:
  explicit DownstreamRoot(LinkStrandRef &&strand);

  bool allows_runtime_child_change() final { return true; }
  ModelRef on_demand_create_child(const Path &path) final {
    return UNAVAILABLE;
  }
};
}

#endif  // DSA_BROKER_DOWNSTREAM_ROOT_H

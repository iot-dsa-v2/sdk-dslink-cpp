#ifndef DSA_BROKER_ROOT_H
#define DSA_BROKER_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {
class RemoteNodeGroup;
class DsBroker;
class BrokerPubRoot;

class BrokerRoot : public NodeModel {
  friend class DsBroker;
  ref_<RemoteNodeGroup> _downstream_root;
  ref_<DsBroker> _broker;
  ref_<BrokerPubRoot> _pub;

  ref_<NodeModel> _sys;

 public:
  BrokerRoot(LinkStrandRef&& strand, ref_<DsBroker>&& broker);
  ~BrokerRoot() override;

  BrokerPubRoot& get_pub() { return *_pub; }
  NodeModel& get_module_root() { return *_sys; }

 protected:
  void destroy_impl() final;
};
}

#endif  // DSA_BROKER_ROOT_H

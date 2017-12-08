#ifndef DSA_BROKER_ROOT_H
#define DSA_BROKER_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {
class DownstreamRoot;
class DsBroker;

class BrokerRoot : public NodeModel {
  friend class DsBroker;
  ref_<DownstreamRoot> _downstream_root;
  ref_<DsBroker> _broker;

 public:
  BrokerRoot(LinkStrandRef &&strand, ref_<DsBroker> &&broker);
  ~BrokerRoot();

 protected:
  void destroy_impl() final;
};
}

#endif  // DSA_BROKER_ROOT_H

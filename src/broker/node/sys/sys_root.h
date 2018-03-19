#ifndef DSA_BROKER_SYS_ROOT_H
#define DSA_BROKER_SYS_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {
class DsBroker;

class BrokerSysRoot : public NodeModel {
 public:
  BrokerSysRoot(const LinkStrandRef &strand, ref_<DsBroker> &&broker);
};
}

#endif  // DSA_BROKER_SYS_ROOT_H

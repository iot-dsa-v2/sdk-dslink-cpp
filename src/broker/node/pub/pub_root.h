#ifndef DSA_BROKER_PUB_ROOT_H
#define DSA_BROKER_PUB_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/profile/pub_root.h"

namespace dsa {

class BrokerPubRoot : public PubRoot {
 public:
  explicit BrokerPubRoot(const LinkStrandRef &strand, const string_ &profile);

  bool allows_runtime_child_change() final { return true; }
};
}

#endif  // DSA_BROKER_PUB_ROOT_H

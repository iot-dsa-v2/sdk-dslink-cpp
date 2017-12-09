#ifndef DSA_BROKER_HOME_ROOT_H
#define DSA_BROKER_HOME_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {

class BrokerHomeRoot : public NodeModel {
 public:
  explicit BrokerHomeRoot(LinkStrandRef &&strand);
};
}

#endif  // DSA_BROKER_HOME_ROOT_H

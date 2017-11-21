#ifndef DSA_BROKER_PUB_ROOT_H
#define DSA_BROKER_PUB_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {

class PubRoot : public NodeModel {
 public:
  explicit PubRoot(LinkStrandRef &&strand);
};
}

#endif  // DSA_BROKER_PUB_ROOT_H
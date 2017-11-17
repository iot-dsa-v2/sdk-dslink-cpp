#ifndef DSA_BROKER_DOWNSTREAM_ROOT_H
#define DSA_BROKER_DOWNSTREAM_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {

class DownstreamRoot : public NodeModel {
 public:
  explicit DownstreamRoot(LinkStrandRef &&strand);
};
}

#endif  // DSA_BROKER_DOWNSTREAM_ROOT_H

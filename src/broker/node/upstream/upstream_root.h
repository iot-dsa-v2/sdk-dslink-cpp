#ifndef DSA_BROKER_UPSTREAM_ROOT_H
#define DSA_BROKER_UPSTREAM_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {

class UpstreamRoot : public NodeModel {
 public:
  explicit UpstreamRoot(LinkStrandRef &&strand);
};
}

#endif  // DSA_BROKER_UPSTREAM_ROOT_H
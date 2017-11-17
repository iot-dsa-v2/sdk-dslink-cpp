#ifndef DSA_BROKER_SYS_ROOT_H
#define DSA_BROKER_SYS_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {

class SysRoot : public NodeModel {
 public:
  explicit SysRoot(LinkStrandRef &&strand);
};
}

#endif  // DSA_BROKER_SYS_ROOT_H

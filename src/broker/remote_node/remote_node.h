#ifndef DSA_BROKER_REMOTE_NODE_H
#define DSA_BROKER_REMOTE_NODE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/model_base.h"

namespace dsa {

class RemoteNode : public NodeModelBase {

public:
  RemoteNode(LinkStrandRef &&strand);
};

}
#endif  // DSA_BROKER_REMOTE_NODE_H

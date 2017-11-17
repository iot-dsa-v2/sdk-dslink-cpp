#ifndef DSA_BROKER_ROOT_H
#define DSA_BROKER_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {

class BrokerRoot : public NodeModel {
  explicit BrokerRoot(LinkStrandRef &&strand);
};

}


#endif //DSA_BROKER_ROOT_H

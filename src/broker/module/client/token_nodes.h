#ifndef DSA_SDK_TOKEN_NODES_H
#define DSA_SDK_TOKEN_NODES_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {

class TokensRoot : public NodeModel {
 public:
  TokensRoot(const LinkStrandRef &strand);
};

class TokenNode : public NodeModel {
 public:
  TokenNode(const LinkStrandRef &strand);

};
}  // namespace dsa

#endif //DSA_SDK_TOKEN_NODES_H

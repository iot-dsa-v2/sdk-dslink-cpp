#include "dsa_common.h"

#include "token_nodes.h"

namespace dsa {
TokensRoot::TokensRoot(const LinkStrandRef &strand) : NodeModel(strand) {}

TokenNode::TokenNode(const LinkStrandRef &strand) : NodeModel(strand) {}
}  // namespace dsa
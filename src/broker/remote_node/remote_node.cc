#include "dsa_common.h"

#include "remote_node.h"

namespace dsa {

RemoteNode::RemoteNode(LinkStrandRef &&strand)
    : NodeModelBase(std::move(strand)) {}
}

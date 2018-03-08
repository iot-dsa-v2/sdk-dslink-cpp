#include "dsa_common.h"

#include "dynamic_children_parent.h"

namespace dsa {

DynamicChildrenParent::DynamicChildrenParent(LinkStrandRef &&strand)
    : NodeModel(std::move(strand)) {}
}

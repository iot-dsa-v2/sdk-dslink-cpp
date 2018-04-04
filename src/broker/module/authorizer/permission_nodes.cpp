#include "dsa_common.h"

#include "permission_nodes.h"

namespace dsa {

PermissionRoleRootNode::PermissionRoleRootNode(const LinkStrandRef &strand)
    : NodeModel(strand) {}
PermissionRoleNode::PermissionRoleNode(const LinkStrandRef &strand,
                                         ref_<NodeModel> &&profile)
    : NodeModel(strand, std::move(profile)) {}

PermissionRuleNode::PermissionRuleNode(const LinkStrandRef &strand,
                                       ref_<NodeModel> &&profile)
    : NodeModel(strand, std::move(profile)) {}
}  // namespace dsa
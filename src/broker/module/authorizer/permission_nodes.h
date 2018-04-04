#ifndef DSA_SDK_PERMISSION_NODES_H
#define DSA_SDK_PERMISSION_NODES_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "responder/node_model.h"

namespace dsa {

class PermissionRoleRootNode : public NodeModel {
 public:
  PermissionRoleRootNode(const LinkStrandRef &strand);
};

class PermissionRoleNode : public NodeModel {
 public:
  PermissionRoleNode(const LinkStrandRef &strand, ref_<NodeModel> &&profile);
};

class PermissionRuleNode : public NodeModel {
 public:
  PermissionRuleNode(const LinkStrandRef &strand, ref_<NodeModel> &&profile);
};

}  // namespace dsa

#endif  // DSA_SDK_PERMISSION_NODES_H

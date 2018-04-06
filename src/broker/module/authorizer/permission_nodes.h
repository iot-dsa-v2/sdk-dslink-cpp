#ifndef DSA_SDK_PERMISSION_NODES_H
#define DSA_SDK_PERMISSION_NODES_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <map>
#include "core/link_strand.h"
#include "responder/node_model.h"
#include "util/enums.h"

namespace dsa {
class BrokerAuthorizer;
class PermissionRuleNode;

class PermissionRoleRootNode : public NodeModel {
 public:
  PermissionRoleRootNode(const LinkStrandRef &strand);
};

class PermissionRoleNode : public NodeModel {
  friend class BrokerAuthorizer;
  friend class PermissionRuleNode;

  // use map to keep it sorted
  std::map<string_, PermissionLevel> _rules;
  PermissionLevel _default_level = PermissionLevel::INVALID;
  ref_<PermissionRoleNode> _fallback;

 public:
  PermissionRoleNode(const LinkStrandRef &strand, ref_<NodeModel> &&profile);
  PermissionLevel get_permission(const string_ &path);

 protected:
  StatusDetail on_set_value(MessageValue &&value) override;
};

class PermissionRuleNode : public NodeModel {
  friend class BrokerAuthorizer;

  ref_<PermissionRoleNode> _role;
  PermissionLevel _level;
  string_ _path;

 public:
  PermissionRuleNode(const LinkStrandRef &strand,
                     ref_<PermissionRoleNode> &&role,
                     ref_<NodeModel> &&profile);

  PermissionLevel get_level() const { return _level; }
  const string_ &get_path() const { return _path; }

 protected:
  StatusDetail on_set_value(MessageValue &&value) override;
};

}  // namespace dsa

#endif  // DSA_SDK_PERMISSION_NODES_H

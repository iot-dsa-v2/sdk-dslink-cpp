#ifndef DSA_SDK_PERMISSION_NODES_H
#define DSA_SDK_PERMISSION_NODES_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <map>
#include "core/link_strand.h"
#include "module/storage.h"
#include "responder/node_model.h"
#include "util/enums.h"

namespace dsa {
class BrokerAuthorizer;
class PermissionRuleNode;
class PermissionRoleNode;
class ValueNodeModel;

class PermissionRoleRootNode : public NodeModel {
  friend class BrokerAuthorizer;
  friend class PermissionRoleNode;
  friend class PermissionRuleNode;

 protected:
  ref_<StrandStorageBucket> _storage;
  void destroy_impl() override;
  void initialize() override;
  void init_default_role_node();

  ref_<PermissionRoleNode> _default_role;

 public:
  PermissionRoleRootNode(const LinkStrandRef &strand);
};

class PermissionRoleNode : public NodeModel {
  friend class BrokerAuthorizer;
  friend class PermissionRuleNode;
  friend class PermissionRoleRootNode;

  // use map to keep it sorted
  std::map<string_, PermissionLevel> _rules;
  PermissionLevel _default_level = PermissionLevel::INVALID;
  ref_<PermissionRoleRootNode> _parent;
  ref_<ValueNodeModel> _fallback_name_node;

 protected:
  string_ _fallback_name;
  ref_<PermissionRoleNode> _fallback_role_cache;
  ref_<PermissionRoleNode> &get_fallback_role();

 public:
  PermissionRoleNode(const LinkStrandRef &strand,
                     ref_<PermissionRoleRootNode> &&parent,
                     ref_<NodeModel> &&profile);
  ~PermissionRoleNode() override;
  PermissionLevel get_permission(const string_ &path, int loop_protect = 16);

 protected:
  void destroy_impl() override;
  StatusDetail on_set_value(MessageValue &&value) override;

  void save_role() const;
  void save_extra(VarMap &map) const override;
  void load_extra(VarMap &map) override;
};

class PermissionRuleNode : public NodeModel {
  friend class BrokerAuthorizer;
  friend class PermissionRoleNode;

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
  void destroy_impl() override;
};

}  // namespace dsa

#endif  // DSA_SDK_PERMISSION_NODES_H

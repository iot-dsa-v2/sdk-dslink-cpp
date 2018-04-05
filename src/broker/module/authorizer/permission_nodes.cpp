#include "dsa_common.h"

#include "permission_nodes.h"

#include "util/string.h"

namespace dsa {

PermissionRoleRootNode::PermissionRoleRootNode(const LinkStrandRef &strand)
    : NodeModel(strand) {}
PermissionRoleNode::PermissionRoleNode(const LinkStrandRef &strand,
                                       ref_<NodeModel> &&profile)
    : NodeModel(strand, std::move(profile)) {}

PermissionLevel PermissionRoleNode::get_permission(const string_ &path) {
  for (auto it = _rules.rbegin(); it != _rules.rend(); ++it) {
    if (str_starts_with(path, it->first)) {
      // math full path or match as folder
      if (path.size() == it->first.size() || path[it->first.size()] == '/') {
        return it->second;
      }
    }
  }
  if (_fallback != nullptr) {
    return _fallback->get_permission(path);
  }
  return PermissionLevel::NONE;
}

PermissionRuleNode::PermissionRuleNode(const LinkStrandRef &strand,
                                       ref_<PermissionRoleNode> &&role,
                                       ref_<NodeModel> &&profile)
    : NodeModel(strand, std::move(profile)), _role(std::move(role)) {}

StatusDetail PermissionRuleNode::on_set_value(MessageValue &&value) {
  if (value.value.is_string()) {
    auto level = PermissionName::parse(value.value.get_string());
    if (level != PermissionLevel::NEVER) {
      if (level != _level) {
        _level = level;
        _role->_rules[_path] = _level;
        return NodeModel::on_set_value(std::move(value));
      }
    }
  }
  return Status::INVALID_PARAMETER;
}
}  // namespace dsa
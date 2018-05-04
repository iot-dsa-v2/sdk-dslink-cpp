#ifndef DSA_SDK_BROKER_AUTHORIZER_H
#define DSA_SDK_BROKER_AUTHORIZER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "module/authorizer.h"

namespace dsa {
class DsBroker;
class PermissionRoleRootNode;
class NodeModel;
class BrokerPubRoot;

class BrokerAuthorizer : public Authorizer {
  friend class DsBroker;

 protected:
  LinkStrandRef _strand;

  ref_<PermissionRoleRootNode> _permission_root;

 public:
  BrokerAuthorizer(const LinkStrandRef& strand);
  ~BrokerAuthorizer();
  void check_permission(const string_& id, const string_& role,
                        const string_& permission_token, MessageType method,
                        const Path& path,
                        CheckPermissionCallback&& callback) override;

  void create_nodes(NodeModel& sys_node, BrokerPubRoot& pub_root);
  ref_<NodeModel> get_permission_root();
};
}  // namespace dsa

#endif  // DSA_SDK_BROKER_AUTHORIZER_H

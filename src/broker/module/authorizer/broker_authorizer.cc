#include "dsa_common.h"

#include "broker_authorizer.h"

#include "../../node/pub/pub_root.h"
#include "permission_nodes.h"

namespace dsa {

BrokerAuthorizer::BrokerAuthorizer(const LinkStrandRef& strand)
    : _strand(strand) {}
BrokerAuthorizer::~BrokerAuthorizer() = default;

ref_<NodeModel> BrokerAuthorizer::get_permission_root() {
  return _permission_root;
}

void BrokerAuthorizer::check_permission(const ClientInfo& client_info,
                                        const string_& permission_token,
                                        MessageType method, const Path& path,
                                        CheckPermissionCallback&& callback) {
  _strand->post([callback = std::move(callback)]() {
    callback(PermissionLevel::CONFIG);
  });
}

void BrokerAuthorizer::create_nodes(NodeModel& module_node,
                                    BrokerPubRoot& pub_root) {
  _permission_root.reset(new PermissionRoleRootNode(_strand));
}

}  // namespace dsa

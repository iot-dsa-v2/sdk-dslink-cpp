#include "dsa_common.h"

#include "broker_authorizer.h"

#include "../../node/pub/pub_root.h"
#include "module/stream_acceptor.h"
#include "permission_nodes.h"
#include "responder/node_state.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "util/string_encode.h"

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
  // init the storage for nodes
  _storage = _strand->storage().get_strand_bucket("Roles", _strand);

  pub_root.register_standard_profile_function(
      "Broker/Permission_Role/Add_Rule",
      (SimpleInvokeNode::FullCallback &&)[this, keepref = get_ref()](
          Var && v, SimpleInvokeNode&, OutgoingInvokeStream & stream,
          ref_<NodeState> && parent) {
        auto* role = parent->model_cast<PermissionRoleNode>();
        if (role != nullptr && v.is_map()) {
          string_ path = v["Path"].to_string();
          string_ permission = v["Permission"].to_string();
          PermissionLevel permisison_level = PermissionName::parse(permission);
          if (permisison_level == PermissionLevel::INVALID) {
            stream.close(Status::INVALID_PARAMETER);
            return;
          }
          if (role->_rules.count(path) > 0) {
            stream.close(Status::INVALID_PARAMETER, "Path is already defined");
            return;
          }
          role->_rules[path] = permisison_level;

          string_ node_name = url_encode_node_name(path);

          auto rule = make_ref_<PermissionRuleNode>(
              _strand, role->get_ref(),
              _strand->stream_acceptor().get_profile("Broker/Permission_Rule",
                                                     true));
          rule->_path = path;
          rule->_level = permisison_level;
          rule->set_value(Var(permission));

          role->add_list_child(node_name, ref_<NodeModelBase>(rule));

          const string_& role_name = role->get_state()->get_path().last_name();
          role->save(*_storage, role_name, false, true);

          stream.close();
        } else {
          stream.close(Status::INVALID_PARAMETER);
        }
      });

  pub_root.register_standard_profile_function(
      "Broker/Permission_Role/Remove",
      (SimpleInvokeNode::FullCallback &&)[this, keepref = get_ref()](
          Var && v, SimpleInvokeNode&, OutgoingInvokeStream & stream,
          ref_<NodeState> && parent) {
        auto* role = parent->model_cast<PermissionRoleNode>();
        if (role != nullptr &&
            role->get_state()->get_parent()->get_model() == _permission_root) {
          const string_& role_name = role->get_state()->get_path().last_name();
          _storage->remove(role_name);
          _permission_root->remove_list_child(role_name);

          stream.close();
        } else {
          stream.close(Status::INVALID_PARAMETER);
        }
      });

  pub_root.register_standard_profile_function(
      "Broker/Permission_Rule/Remove",
      (SimpleInvokeNode::FullCallback &&)[this, keepref = get_ref()](
          Var && v, SimpleInvokeNode&, OutgoingInvokeStream & stream,
          ref_<NodeState> && parent) {
        auto* rule = parent->model_cast<PermissionRuleNode>();
        if (rule != nullptr) {
          auto* role = parent->get_parent()->model_cast<PermissionRoleNode>();
          if (role != nullptr) {
            const string_& rule_name =
                rule->get_state()->get_path().last_name();
            const string_& role_name =
                role->get_state()->get_path().last_name();

            role->_rules.erase(rule->_path);
            role->save(*_storage, role_name, false, true);
            role->remove_list_child(rule_name);
            stream.close();
            return;
          }
        }
        stream.close(Status::INVALID_PARAMETER);
      });

  _permission_root.reset(new PermissionRoleRootNode(_strand));
}

}  // namespace dsa

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

void BrokerAuthorizer::check_permission(const string_& id, const string_& role,
                                        const string_& permission_token,
                                        MessageType method, const Path& path,
                                        CheckPermissionCallback&& callback) {
  _strand->post([
    this, keepref = get_ref(), role, path, callback = std::move(callback)
  ]() {
    if (role.empty()) {
      callback(
          _permission_root->_default_role->get_permission(path.full_str()));
      return;
    }
    auto* p_role = dynamic_cast<PermissionRoleNode*>(
        _permission_root->get_child(role).get());
    if (p_role != nullptr) {
      callback(p_role->get_permission(path.full_str()));
    } else {
      callback(
          _permission_root->_default_role->get_permission(path.full_str()));
    }
  });
}

void BrokerAuthorizer::create_nodes(NodeModel& sys_node,
                                    BrokerPubRoot& pub_root) {
  pub_root.register_standard_profile_function(
      "Broker/Permission_Role/Add_Rule",
      CAST_LAMBDA(SimpleInvokeNode::FullCallback)[this, keepref = get_ref()](
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

          string_ node_name = url_encode_node_name(path);

          if (role->get_child(node_name) != nullptr) {
            stream.close(Status::INVALID_PARAMETER, "Path is already defined");
            return;
          }

          role->_rules[path] = permisison_level;

          auto rule = make_ref_<PermissionRuleNode>(
              _strand, role->get_ref(),
              _strand->stream_acceptor().get_profile("Broker/Permission_Rule",
                                                     true));
          rule->_path = path;
          rule->_level = permisison_level;
          rule->set_value(Var(permission));

          role->add_list_child(node_name, ref_<NodeModelBase>(rule));

          const string_& role_name = role->get_state()->get_path().node_name();
          role->save_role();

          stream.close();
        } else {
          stream.close(Status::INVALID_PARAMETER);
        }
      });

  pub_root.register_standard_profile_function(
      "Broker/Permission_Role/Remove",
      CAST_LAMBDA(SimpleInvokeNode::FullCallback)[this, keepref = get_ref()](
          Var && v, SimpleInvokeNode&, OutgoingInvokeStream & stream,
          ref_<NodeState> && parent) {
        auto* role = parent->model_cast<PermissionRoleNode>();
        if (role != nullptr &&
            role->get_state()->get_parent()->get_model() == _permission_root) {
          const string_& role_name = role->get_state()->get_path().node_name();
          _permission_root->_storage->remove(role_name);
          _permission_root->remove_list_child(role_name);

          stream.close();
        } else {
          stream.close(Status::INVALID_PARAMETER);
        }
      });

  pub_root.register_standard_profile_function(
      "Broker/Permission_Rule/Remove",
      CAST_LAMBDA(SimpleInvokeNode::FullCallback)[this, keepref = get_ref()](
          Var && v, SimpleInvokeNode&, OutgoingInvokeStream & stream,
          ref_<NodeState> && parent) {
        auto* rule = parent->model_cast<PermissionRuleNode>();
        if (rule != nullptr) {
          auto* role = parent->get_parent()->model_cast<PermissionRoleNode>();
          if (role != nullptr) {
            const string_& rule_name =
                rule->get_state()->get_path().node_name();
            const string_& role_name =
                role->get_state()->get_path().node_name();

            role->_rules.erase(rule->_path);
            role->save_role();
            role->remove_list_child(rule_name);
            stream.close();
            return;
          }
        }
        stream.close(Status::INVALID_PARAMETER);
      });

  _permission_root.reset(new PermissionRoleRootNode(_strand));
  _permission_root->add_list_child(
      "Add_Role",
      make_ref_<SimpleInvokeNode>(
          _strand, [ this, keepref = get_ref() ](Var && v)->Var {
            if (_permission_root != nullptr && v.is_map()) {
              string_ name = v["Name"].to_string();
              if (name.empty()) {
                return Var(Status::INVALID_PARAMETER);
              }
              if (_permission_root->get_list_children().count(name) > 0) {
                return Var(Status::INVALID_PARAMETER,
                           "Name of role is already defined");
              }
              if (PathData::invalid_name(name)) {
                return Var(Status::INVALID_PARAMETER, "Name is invalid");
              }
              auto role = make_ref_<PermissionRoleNode>(
                  _strand, _permission_root->get_ref(),
                  _strand->stream_acceptor().get_profile(
                      "Broker/Permission_Role", true));
              _permission_root->add_list_child(name, std::move(role));
              return Var(Status::DONE);
            } else {
              return Var(Status::INVALID_PARAMETER);
            }
          },
          PermissionLevel::CONFIG));
}

}  // namespace dsa

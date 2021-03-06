#include "dsa_common.h"

#include "broker_client_manager.h"

#include "../../node/paths.h"
#include "../../node/pub/pub_root.h"
#include "../../remote_node/broker_session_manager.h"
#include "../../remote_node/dynamic_children_parent.h"
#include "broker_client_nodes.h"
#include "crypto/hash.h"
#include "crypto/misc.h"
#include "module/logger.h"
#include "module/stream_acceptor.h"
#include "quaratine_node.h"
#include "responder/node_state.h"
#include "responder/node_state_manager.h"
#include "responder/value_node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "token_nodes.h"
#include "util/string.h"

namespace dsa {

BrokerClientManager::BrokerClientManager(const LinkStrandRef& strand)
    : _strand(strand) {
  _config = make_ref_<BrokerClientManagerConfig>();
  _allow_all_links = _config->allow_all_links().get_value().get_bool();
  _quarantine_enabled = _config->enable_quarantine().get_value().get_bool();
}
BrokerClientManager::~BrokerClientManager() = default;

ref_<NodeModel> BrokerClientManager::get_clients_root() {
  return _clients_root;
}
ref_<NodeModel> BrokerClientManager::get_quarantine_root() {
  return _quarantine_root;
}
ref_<NodeModel> BrokerClientManager::get_tokens_root() { return _tokens_root; }

StatusDetail BrokerClientManager::update_client_path(const string_& dsid,
                                                     const string_& new_path) {
  auto& children = _clients_root->get_list_children();

  auto search = children.find(dsid);
  if (search == children.end()) {
    return Status::INTERNAL_ERROR;  // shouldn't happen
  }
  auto* p_client_model = dynamic_cast<BrokerClientNode*>(search->second.get());
  if (p_client_model == nullptr) {
    return Status::INTERNAL_ERROR;  // shouldn't happen
  }
  if (!new_path.empty()) {
    if (!str_starts_with(new_path, DOWNSTREAM_PATH)) {
      return {Status::INVALID_PARAMETER,
              "Path should start with " + DOWNSTREAM_PATH};
    }
    Path path(new_path);
    if (path.data()->names.size() != 2) {
      return {Status::INVALID_PARAMETER,
              "Path should start with " + DOWNSTREAM_PATH};
    }

    auto state = _downstream->get_child(path.data()->names[1], true);

    if (state->get_model() != nullptr) {
      return {Status::INVALID_PARAMETER, "Path already in use"};
    }
  }

  static_cast<BrokerSessionManager&>(_strand->session_manager())
      .update_responder_root(
          dsid, p_client_model->get_client_info().responder_path, new_path);

  return StatusDetail();
}
void BrokerClientManager::create_nodes(NodeModel& module_node,
                                       BrokerPubRoot& pub_root) {
  _downstream = static_cast<NodeStateManager&>(_strand->stream_acceptor())
                    .check_state(Path(DOWNSTREAM_NAME));

  pub_root.register_standard_profile_function(
      "broker/client/remove",
      [ this, keepref = get_ref() ](Var&&, SimpleInvokeNode&,
                                    OutgoingInvokeStream & stream,
                                    ref_<NodeState> && parent) {
        auto* client = parent->model_cast<BrokerClientNode>();
        if (client != nullptr &&
            parent->get_parent() == _clients_root->get_state()) {
          const string_ dsid = parent->get_path().node_name();
          const string_ responder_path =
              client->get_client_info().responder_path;

          // delete the storage
          _clients_root->_storage->remove(dsid);
          // remove the node
          _clients_root->remove_list_child(dsid);

          stream.close();

          static_cast<BrokerSessionManager&>(_strand->session_manager())
              .remove_sessions(dsid, responder_path);

        } else {
          stream.close(Status::INVALID_PARAMETER);
        }
      });

  pub_root.register_standard_profile_function(
      "broker/client/detach-token",
      CAST_LAMBDA(SimpleInvokeNode::FullCallback)[this, keepref = get_ref()](
          Var&&, SimpleInvokeNode&, OutgoingInvokeStream & stream,
          ref_<NodeState> && parent) {
        auto* client = parent->model_cast<BrokerClientNode>();
        if (client != nullptr) {
          if (client->detach_token()) {
            client->save(*_clients_root->_storage,
                         client->get_client_info().id);
          }
        } else {
          stream.close(Status::INVALID_PARAMETER);
        }
      });

  pub_root.register_standard_profile_function(
      "broker/quarantine-client/authorize",
      CAST_LAMBDA(SimpleInvokeNode::FullCallback)[this, keepref = get_ref()](
          Var && v, SimpleInvokeNode&, OutgoingInvokeStream & stream,
          ref_<NodeState> && parent) {
        auto* quarantine_root = parent->model_cast<QuaratineRemoteRoot>();
        if (quarantine_root != nullptr && v.is_map()) {
          const string_& dsid =
              quarantine_root->get_state()->get_path().node_name();
          string_ role;
          if (v["Role"].is_string()) {
            role = v["Role"].get_string();
          }
          int64_t max_session = 1;
          if (v["Max Session"].is_int()) {
            max_session = v["Max Session"].get_int();
            if (max_session < 1) {
              stream.close(Status::INVALID_PARAMETER, "invalid Max Session");
              return;
            }
          }

          Var& v_path = v["Path"];
          string_ path;
          if (v_path.is_string()) {
            path = v_path.get_string();
          }
          if (path.empty()) {
            path = create_downstream_path(dsid);
          } else if (max_session == 1) {
            // TODO, pure requester without responder path
            // but still limited to 1 max_session??
            if (!str_starts_with(path, DOWNSTREAM_PATH)) {
              stream.close(Status::INVALID_PARAMETER, "invalid Path");
              return;
            }
            string_ downstream_name = path.substr(DOWNSTREAM_PATH.size());
            auto state = _downstream->get_child(downstream_name, true);

            if (state->get_model() != nullptr) {
              stream.close(Status::INVALID_PARAMETER, "Path is already in use");
              return;
            }
          }
          ClientInfo info(dsid);
          info.responder_path = path;
          info.max_session = max_session;
          info.role = role;

          auto child = make_ref_<BrokerClientNode>(
              _strand, _clients_root->get_ref(),
              _strand->stream_acceptor().get_profile("broker/client", true),
              dsid);
          child->set_client_info(std::move(info));

          _clients_root->add_list_child(dsid, ref_<NodeModelBase>(child));
          child->save(*_clients_root->_storage, dsid, false, true);

          // remove the existing connection as well as the node
          static_cast<BrokerSessionManager&>(_strand->session_manager())
              .remove_sessions(
                  dsid, quarantine_root->get_state()->get_path().full_str());
          stream.close();
        } else {
          stream.close(Status::INVALID_PARAMETER);
        }
      });

  pub_root.register_standard_profile_function(
      "broker/token/remove",
      CAST_LAMBDA(SimpleInvokeNode::FullCallback)[this, keepref = get_ref()](
          Var&&, SimpleInvokeNode&, OutgoingInvokeStream & stream,
          ref_<NodeState> && parent) {
        auto* token = parent->model_cast<TokenNode>();
        if (token != nullptr &&
            parent->get_parent() == _tokens_root->get_state()) {
          if (token->_managed) {
            token->remove_all_clients();
          }
          string_ token_name = parent->get_path().node_name();
          _tokens_root->_storage->remove(token_name);
          _tokens_root->remove_list_child(token_name);
          stream.close();
        } else {
          stream.close(Status::INVALID_PARAMETER);
        }
      });
  pub_root.register_standard_profile_function(
      "broker/token/remove-all-clients",
      CAST_LAMBDA(SimpleInvokeNode::FullCallback)[this, keepref = get_ref()](
          Var&&, SimpleInvokeNode&, OutgoingInvokeStream & stream,
          ref_<NodeState> && parent) {
        auto* token = parent->model_cast<TokenNode>();
        if (token != nullptr &&
            parent->get_parent() == _tokens_root->get_state()) {
          token->remove_all_clients();
          stream.close();
        } else {
          stream.close(Status::INVALID_PARAMETER);
        }
      });
  pub_root.register_standard_profile_function(
      "broker/token/regenerate",
      CAST_LAMBDA(SimpleInvokeNode::FullCallback)[this, keepref = get_ref()](
          Var&&, SimpleInvokeNode&, OutgoingInvokeStream & stream,
          ref_<NodeState> && parent) {
        auto* token = parent->model_cast<TokenNode>();
        if (token != nullptr &&
            parent->get_parent() == _tokens_root->get_state()) {
          token->regenerate();
          stream.close();
        } else {
          stream.close(Status::INVALID_PARAMETER);
        }
      });

  _clients_root.reset(new BrokerClientsRoot(_strand, get_ref()));
  _quarantine_root.reset(new QuaratineRoot(_strand));
  _tokens_root.reset(new TokensRoot(_strand, get_ref()));

  _clients_root
      ->add_list_child(
          "allow-all",
          make_ref_<ValueNodeModel>(
              _strand, "bool",
              [ this, keepref = get_ref() ](const Var& v)->StatusDetail {
                if (v.is_bool()) {
                  set_allow_all_links(v.get_bool());
                  return Status::DONE;
                }
                return Status::INVALID_PARAMETER;
              },
              PermissionLevel::CONFIG))
      ->set_value(Var(_allow_all_links));

  _quarantine_root
      ->add_list_child(
          "enabled",
          make_ref_<ValueNodeModel>(
              _strand, "bool",
              [ this, keepref = get_ref() ](const Var& v)->StatusDetail {
                if (v.is_bool()) {
                  set_quarantine_enabled(v.get_bool());
                  return Status::DONE;
                }
                return Status::INVALID_PARAMETER;
              },
              PermissionLevel::CONFIG))
      ->set_value(Var(_quarantine_enabled));
}

void BrokerClientManager::remove_clients_from_token(const string_& token_name) {
  auto clients_node = _clients_root->get_list_children();
  std::vector<std::tuple<string_, BrokerClientNode*>> to_remove;
  for (auto& it : _clients_root->get_list_children()) {
    BrokerClientNode* p = dynamic_cast<BrokerClientNode*>(it.second.get());
    if (p != nullptr && p->get_client_info().from_token == token_name) {
      to_remove.push_back({it.first, p});
    }
  }
  string_ dsid;
  BrokerClientNode* client_node;
  for (auto& it : to_remove) {
    std::tie(dsid, client_node) = it;
    const string_ responder_path =
        client_node->get_client_info().responder_path;

    // delete the storage
    _clients_root->_storage->remove(dsid);
    // remove the node
    _clients_root->remove_list_child(dsid);

    static_cast<BrokerSessionManager&>(_strand->session_manager())
        .remove_sessions(dsid, responder_path);
  }
}

void BrokerClientManager::get_client(const string_& id,
                                     const string_& auth_token,
                                     bool is_responder,
                                     ClientInfo::GetClientCallback&& callback) {
  // make the callback async so all the implementation will have same behavior
  _strand->post([
    this, keepref = get_ref(), id, auth_token, callback = std::move(callback),
    is_responder
  ]() {
    if (PathData::invalid_name(id)) {
      callback(ClientInfo(id), true);
      return;
    }
    if (id.size() < 43) {
      // requester only clients
      // split token, find the first '/'
      // (it was originally ';', converted to '/' by the server connection)
      auto pos = auth_token.find('/');
      if (pos == string_::npos) {
        // invalid token
        callback(ClientInfo(id), true);
      }
      string_ permission_role = auth_token.substr(0, pos);

      auto search = _clients_root->get_list_children().find(id);

      if (search != _clients_root->get_list_children().end()) {
        // a known dslink

        auto* p = dynamic_cast<BrokerClientNode*>(search->second.get());
        if (p != nullptr) {
          if (p->temporary_client &&
              p->get_client_info().role != permission_role) {
            auto copy_info = p->get_client_info();
            copy_info.role = permission_role;
            p->set_client_info(std::move(copy_info));
            // todo. kick all clients because permission is different?
          }
          callback(p->get_client_info(), false);
        } else {
          // probably a userid conflict with action node name
          callback(ClientInfo(id), true);
        }
      } else {
        // new client
        ClientInfo info(id);
        info.max_session = 0xFFFFFFFF;
        info.role = permission_role;

        // add to downstream
        auto child = make_ref_<BrokerClientNode>(
            _strand, _clients_root->get_ref(),
            _strand->stream_acceptor().get_profile("broker/client", true), id);
        child->temporary_client = true;
        child->set_client_info(std::move(info));

        _clients_root->add_list_child(id, ref_<NodeModelBase>(child));
        // do not save temporary client to disck
        // child->save(*_clients_root->_storage, id, false, true);

        callback(child->get_client_info(), false);
      }
    } else {
      // normal clients with public/private key

      auto search = _clients_root->get_list_children().find(id);

      if (search != _clients_root->get_list_children().end()) {
        // a known dslink

        auto* p = dynamic_cast<BrokerClientNode*>(search->second.get());
        if (p != nullptr) {
          callback(p->get_client_info(), false);
        } else {
          // this doesn't make sense, a dsId conflict with action node name?
          callback(ClientInfo(id), true);
        }
      } else {
        // unknown dslink

        // token is 59 character, 16 bytes token name, and 43 bytes base64 hash
        if (auth_token.length() == 59) {
          // check if token is valid
          auto token_name = auth_token.substr(0, 16);
          auto token_node = static_cast<TokenNode*>(
              _tokens_root->get_child(token_name).get());
          // find the token
          if (token_node != nullptr && token_node->is_valid()) {
            if (validate_token_auth(id, token_node->_token,
                                    auth_token.substr(16))) {
              // token is valid ! create a client for this id
              ClientInfo info(id);
              info.role = token_node->_role;
              info.from_token = token_name;
              info.max_session = token_node->_max_session;
              if (is_responder && info.max_session == 1) {
                info.responder_path = create_downstream_path(id);
              }

              if (token_node->_count > 0) {
                // the token is used
                token_node->_count--;
                token_node->save_token();
              }

              // add to downstream
              auto child = make_ref_<BrokerClientNode>(
                  _strand, _clients_root->get_ref(),
                  _strand->stream_acceptor().get_profile("broker/client", true),
                  id);
              child->set_client_info(std::move(info));

              _clients_root->add_list_child(id, ref_<NodeModelBase>(child));
              child->save(*_clients_root->_storage, id, false, true);

              callback(child->get_client_info(), false);
              return;
            }
          }
          callback(ClientInfo(id), true);
        } else if (_allow_all_links) {
          // check if all links are allowed by default
          // setting this to true means you don't care about security
          ClientInfo info(id);
          if (is_responder) {
            info.responder_path = create_downstream_path(id);
          }

          // add to downstream
          auto child = make_ref_<BrokerClientNode>(
              _strand, _clients_root->get_ref(),
              _strand->stream_acceptor().get_profile("broker/client", true),
              id);
          child->set_client_info(std::move(info));

          _clients_root->add_list_child(id, ref_<NodeModelBase>(child));
          child->save(*_clients_root->_storage, id, false, true);

          callback(child->get_client_info(), false);
        } else if (_quarantine_enabled) {
          ClientInfo info(id);
          info.role = "none";
          info.responder_path = QUARANTINE_PATH + id;
          callback(std::move(info), false);
        } else {
          callback(ClientInfo(id), true);
        }
      }
    }
  });
}
string_ BrokerClientManager::create_downstream_path(const string_& dsid) {
  int start_len = static_cast<int>(dsid.length()) - 43;
  if (start_len < 3) start_len = 3;
  if (dsid[start_len - 1] == '-') {
    start_len--;
  }

  for (; start_len < dsid.length(); ++start_len) {
    string_ name = dsid.substr(0, start_len);
    auto state = _downstream->get_child(name, true);

    if (state->get_model() == nullptr) {
      return state->get_path().full_str();
    }
  }
  LOG_FATAL(__FILENAME__, LOG << "impossible conflict of dsid" << dsid);
}

void BrokerClientManager::set_allow_all_links(bool value) {
  if (value != _allow_all_links) {
    _allow_all_links = value;
    _config->allow_all_links().set_value(Var(_allow_all_links));
    _config->save();
  }
}
void BrokerClientManager::set_quarantine_enabled(bool value) {
  if (value != _quarantine_enabled) {
    _quarantine_enabled = value;
    _config->enable_quarantine().set_value(Var(_quarantine_enabled));
    _config->save();
  }
}

void BrokerClientManager::destroy_impl() {
  _downstream.reset();
  _clients_root.reset();
  _tokens_root.reset();
  _quarantine_root.reset();
  ClientManager::destroy_impl();
}

}  // namespace dsa

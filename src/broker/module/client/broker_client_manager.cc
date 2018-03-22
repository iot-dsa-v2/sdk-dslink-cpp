#include "dsa_common.h"

#include "broker_client_manager.h"

#include "../../node/paths.h"
#include "../../node/pub/pub_root.h"
#include "../../remote_node/broker_session_manager.h"
#include "../../remote_node/dynamic_children_parent.h"
#include "broker_client_nodes.h"
#include "module/logger.h"
#include "module/stream_acceptor.h"
#include "quaratine_node.h"
#include "responder/node_state.h"
#include "responder/value_node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"
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
void BrokerClientManager::rebuild_path2id() {
  if (is_destroyed()) return;

  _path2id.clear();
  for (auto& it : _clients_root->get_list_children()) {
    auto* link_node = dynamic_cast<BrokerClientNode*>(it.second.get());
    if (link_node != nullptr) {
      if (!link_node->get_client_info().id.empty())
        _path2id[link_node->get_client_info().responder_path.substr(
            DOWNSTREAM_PATH.size())] = it.first;
    }
  }
}

string_ BrokerClientManager::update_client_path(const string_& dsid,
                                                const string_& new_path) {
  auto& children = _clients_root->get_list_children();

  auto search = children.find(dsid);
  if (search == children.end()) {
    return "internal error";  // shouldn't happen
  }
  auto* p_client_model = dynamic_cast<BrokerClientNode*>(search->second.get());
  if (p_client_model == nullptr) {
    return "internal error";  // shouldn't happen
  }
  if (!new_path.empty()) {
    if (!str_starts_with(new_path, DOWNSTREAM_PATH)) {
      return "Path should start with " + DOWNSTREAM_PATH;
    }
    Path path(new_path);
    if (path.data()->names.size() != 2) {
      return "err";
    }

    if (_path2id.count(path.data()->names[1]) > 0) {
      return "Path already in use";
    }
  }

  static_cast<BrokerSessionManager&>(_strand->session_manager())
      .update_responder_root(
          dsid, p_client_model->get_client_info().responder_path, new_path);

  return "";
}
void BrokerClientManager::create_nodes(NodeModel& module_node,
                                       BrokerPubRoot& pub_root) {
  pub_root.register_standard_profile_function(
      "Broker/Client/Remove",
      [ this, keepref = get_ref() ](Var&&, SimpleInvokeNode&,
                                    OutgoingInvokeStream & stream,
                                    ref_<NodeState> && parent) {
        auto* client = parent->model_cast<BrokerClientNode>();
        if (client != nullptr &&
            parent->get_parent() == _clients_root->get_state()) {
          const string_ dsid = parent->get_path().last_name();
          const string_ responder_path =
              client->get_client_info().responder_path;

          // remove from path2id map
          if (str_starts_with(responder_path, DOWNSTREAM_PATH)) {
            _path2id.erase(responder_path.substr(responder_path.size()));
          }

          // delete the storage
          _clients_root->_storage->remove(dsid);
          // remove the node
          _clients_root->remove_list_child(dsid);

          stream.close();

          static_cast<BrokerSessionManager&>(_strand->session_manager())
              .remove_sessions(dsid, responder_path);

        } else {
          stream.close(MessageStatus::INVALID_PARAMETER);
        }
      });

  pub_root.register_standard_profile_function(
      "Broker/Client/Detach_Token",
      [ this, keepref = get_ref() ](Var&&, SimpleInvokeNode&,
                                    OutgoingInvokeStream & stream,
                                    ref_<NodeState> && parent) {
        auto* client = parent->model_cast<BrokerClientNode>();
        if (client != nullptr) {
          client->detach_token();
          client->save(*_clients_root->_storage, client->get_client_info().id);
        } else {
          stream.close(MessageStatus::INVALID_PARAMETER);
        }
      });

  pub_root.register_standard_profile_function(
      "Broker/Quarantine_Client/Authorize",
      [ this, keepref = get_ref() ](Var && v, SimpleInvokeNode&,
                                    OutgoingInvokeStream & stream,
                                    ref_<NodeState> && parent) {
        auto* quarantine_root = parent->model_cast<QuaratineRemoteRoot>();
        if (quarantine_root != nullptr && v.is_map()) {
          const string_& dsid =
              quarantine_root->get_state()->get_path().last_name();
          string_ group;
          if (v["Group"].is_string()) {
            group = v["Group"].get_string();
          }
          int64_t max_session = 1;
          if (v["Max_Session"].is_int()) {
            max_session = v["Max_Session"].get_int();
            if (max_session < 1) {
              stream.close(MessageStatus::INVALID_PARAMETER,
                           "invalid Max_Session");
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
              stream.close(MessageStatus::INVALID_PARAMETER, "invalid Path");
              return;
            }
            string_ downstream_name = path.substr(DOWNSTREAM_PATH.size());
            if (_path2id.count(downstream_name) > 0) {
              stream.close(MessageStatus::INVALID_PARAMETER,
                           "Path is already in use");
              return;
            }
            _path2id[downstream_name] = dsid;
          }
          ClientInfo info(dsid);
          info.responder_path = path;
          info.max_session = max_session;
          info.group = group;

          auto child = make_ref_<BrokerClientNode>(
              _strand, _clients_root->get_ref(),
              _strand->stream_acceptor().get_profile("Broker/Client", true),
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
          stream.close(MessageStatus::INVALID_PARAMETER);
        }
      });

  _clients_root.reset(new BrokerClientsRoot(_strand, get_ref()));
  _quarantine_root.reset(new QuaratineRoot(_strand));
  _tokens_root.reset(new NodeModel(_strand));

  _clients_root
      ->add_list_child(
          "Allow_All",
          make_ref_<ValueNodeModel>(
              _strand, "bool", [ this, keepref = get_ref() ](const Var& v) {
                if (v.is_bool()) {
                  set_allow_all_links(v.get_bool());
                  return true;
                }
                return false;
              },
              PermissionLevel::CONFIG))
      ->set_value(Var(_allow_all_links));

  _quarantine_root
      ->add_list_child(
          "Enabled",
          make_ref_<ValueNodeModel>(
              _strand, "bool", [ this, keepref = get_ref() ](const Var& v) {
                if (v.is_bool()) {
                  set_quarantine_enabled(v.get_bool());
                  return true;
                }
                return false;
              },
              PermissionLevel::CONFIG))
      ->set_value(Var(_quarantine_enabled));
}

void BrokerClientManager::get_client(const string_& dsid,
                                     const string_& auth_token,
                                     bool is_responder,
                                     ClientInfo::GetClientCallback&& callback) {
  // make the callback async so all the implementation will have same behavior
  _strand->post([
    this, keepref = get_ref(), dsid, auth_token, callback = std::move(callback),
    is_responder
  ]() {
    if (PathData::invalid_name(dsid)) {
      // TODO check dsid
      callback(ClientInfo(dsid), true);
      return;
    }

    auto search = _clients_root->get_list_children().find(dsid);

    if (search != _clients_root->get_list_children().end()) {
      // a known dslink

      auto* p = dynamic_cast<BrokerClientNode*>(search->second.get());
      if (p != nullptr) {
        callback(p->get_client_info(), false);
      } else {
        // this doesn't make sense, a dsId conflict with action node name?
        callback(ClientInfo(dsid), true);
      }
    } else {
      // unknown dslink

      // TODO check token first
      if (_allow_all_links) {
        ClientInfo info(dsid);
        if (is_responder) {
          info.responder_path = create_downstream_path(dsid);
        }

        // add to downstream
        auto child = make_ref_<BrokerClientNode>(
            _strand, _clients_root->get_ref(),
            _strand->stream_acceptor().get_profile("Broker/Client", true),
            dsid);
        child->set_client_info(std::move(info));

        _clients_root->add_list_child(dsid, ref_<NodeModelBase>(child));
        child->save(*_clients_root->_storage, dsid, false, true);

        callback(child->get_client_info(), false);
      } else if (_quarantine_enabled) {
        ClientInfo info(dsid);
        info.group = "none";
        info.responder_path = QUARANTINE_PATH + dsid;
        callback(std::move(info), false);
      } else {
        callback(ClientInfo(dsid), true);
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
    if (_path2id.count(name) == 0) {
      _path2id[name] = dsid;
      return DOWNSTREAM_PATH + name;
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
  _clients_root.reset();
  _quarantine_root.reset();
  ClientManager::destroy_impl();
}

}  // namespace dsa

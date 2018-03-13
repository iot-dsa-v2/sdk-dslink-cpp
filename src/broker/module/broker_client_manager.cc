#include "dsa_common.h"

#include "broker_client_manager.h"

#include "../node/paths.h"
#include "../node/pub/pub_root.h"
#include "../remote_node/broker_session_manager.h"
#include "../remote_node/dynamic_children_parent.h"
#include "broker_client_nodes.h"
#include "module/logger.h"
#include "module/stream_acceptor.h"
#include "responder/node_state.h"
#include "responder/value_node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "util/string.h"

namespace dsa {

BrokerClientManager::BrokerClientManager(LinkStrandRef& strand)
    : _strand(strand) {
  _config = make_ref_<BrokerClientManagerConfig>();
  _allow_all_links = _config->allow_all_links().get_value().get_bool();
  _quarantine_enabled = _config->enable_quarantine().get_value().get_bool();
}
BrokerClientManager::~BrokerClientManager() = default;
ref_<NodeModel> BrokerClientManager::get_clients_node() { return _clients; }

void BrokerClientManager::rebuild_path2id() {
  if (is_destroyed()) return;

  _path2id.clear();
  for (auto& it : _clients->get_list_children()) {
    auto* link_node = dynamic_cast<BrokerClientNode*>(it.second.get());
    if (link_node != nullptr) {
      if (!link_node->get_client_info().id.empty())
        _path2id[link_node->get_client_info().id] = it.first;
    }
  }
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
            parent->get_parent() == _clients->get_state()) {
          const string_& dsid = parent->get_path().last_name();
          const string_& responder_path =
              client->get_client_info().responder_path;

          // remove from path2id map
          if (str_starts_with(responder_path, DOWNSTREAM_PATH)) {
            _path2id.erase(responder_path.substr(responder_path.size()));
          }

          // delete the storage
          _clients->_storage->remove(dsid);
          // remove the node
          _clients->remove_child(dsid);

          stream.close();

          static_cast<BrokerSessionManager&>(_strand->session_manager())
              .remove_sessions(dsid, responder_path);

        } else {
          stream.close(MessageStatus::INVALID_PARAMETER);
        }
      });

  _clients.reset(new BrokerClientsRoot(_strand->get_ref(), get_ref()));
  _quarantine.reset(new DynamicChildrenParent(_strand->get_ref()));

  _quarantine->add_list_child(
      "Allow_All",
      make_ref_<ValueNodeModel>(_strand->get_ref(),
                                [ this, keepref = get_ref() ](const Var& v) {
                                  if (v.is_bool()) {
                                    set_allow_all_links(v.get_bool());
                                    return true;
                                  }
                                  return false;
                                },
                                PermissionLevel::CONFIG));

  _quarantine->add_list_child(
      "Enabled",
      make_ref_<ValueNodeModel>(_strand->get_ref(),
                                [ this, keepref = get_ref() ](const Var& v) {
                                  if (v.is_bool()) {
                                    set_quarantine_enabled(v.get_bool());
                                    return true;
                                  }
                                  return false;
                                },
                                PermissionLevel::CONFIG));
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
      callback(ClientInfo(dsid, auth_token), true);
      return;
    }

    auto search = _clients->get_list_children().find(dsid);

    if (search != _clients->get_list_children().end()) {
      // a known dslink

      auto* p = dynamic_cast<BrokerClientNode*>(search->second.get());
      if (p != nullptr) {
        callback(p->get_client_info(), false);
      } else {
        // this doesn't make sense, a dsId conflict with action node name?
        callback(ClientInfo(dsid, auth_token), true);
      }
    } else {
      // unknown dslink

      // TODO check token first
      if (_allow_all_links) {
        ClientInfo info(dsid, auth_token);
        if (is_responder) {
          info.responder_path = create_downstream_path(dsid);
        }

        // add to downstream
        auto child = make_ref_<BrokerClientNode>(
            _strand->get_ref(),
            _strand->stream_acceptor().get_profile("Broker/Client", true));
        // TODO, add more into constructor, allow value node change property,
        // add group
        child->get_client_info() = std::move(info);

        _clients->add_list_child(dsid, ref_<NodeModelBase>(child));
        child->save(*_clients->_storage, dsid, false, true);

        callback(child->get_client_info(), false);
      } else if (_quarantine_enabled) {
        // TODO add to quaratine
      } else {
        callback(ClientInfo(dsid, auth_token), true);
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
      return "Downstream/" + name;
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
  _clients.reset();
  _quarantine.reset();
  ClientManager::destroy_impl();
}

}  // namespace dsa

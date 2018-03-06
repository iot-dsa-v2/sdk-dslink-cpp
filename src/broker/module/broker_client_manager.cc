#include "dsa_common.h"

#include "broker_client_manager.h"

#include "broker_client_nodes.h"
#include "module/logger.h"
#include "module/stream_acceptor.h"
#include "responder/value_node_model.h"

namespace dsa {

void BrokerClientManager::rebuild_path2id() {
  _path2id.clear();
  for (auto& it : _clients->get_list_children()) {
    auto* link_node = dynamic_cast<BrokerClientNode*>(it.second.get());
    if (link_node != nullptr) {
    }
  }
}

void BrokerClientManager::create_nodes(NodeModel& module_node,
                                       BrokerPubRoot& pub_root) {
  // TODO register action for pub root

  _clients.reset(new BrokerClientsRoot(_strand->get_ref()));
  _quarantine.reset(new NodeModel(_strand->get_ref()));

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
  _strand->post([
    this, keepref = get_ref(), dsid, auth_token, callback = std::move(callback),
    is_responder
  ]() {
    auto search = _clients->get_list_children().find(dsid);

    if (PathData::invalid_name(dsid)) {
      // TODO check dsid
      callback(ClientInfo(dsid, auth_token), true);
      return;
    }

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
  LOG_FATAL("BrokerClientManager",
            LOG << "impossible conflict of dsid" << dsid);
}

void BrokerClientManager::set_allow_all_links(bool value) {
  if (value != _allow_all_links) {
    _allow_all_links = value;
  }
}
void BrokerClientManager::set_quarantine_enabled(bool value) {
  if (value != _quarantine_enabled) {
    _quarantine_enabled = value;
  }
}

void BrokerClientManager::destroy_impl() {
  _clients.reset();
  _quarantine.reset();
  ClientManager::destroy_impl();
}
}  // namespace dsa

#include "dsa_common.h"

#include "broker_client_manager.h"
#include "broker_client_nodes.h"
#include "module/session_manager.h"
#include "module/storage.h"
#include "module/stream_acceptor.h"
#include "responder/node_state.h"
#include "responder/value_node_model.h"

namespace dsa {

BrokerClientsRoot::BrokerClientsRoot(LinkStrandRef&& strand,
                                     ref_<BrokerClientManager>&& manager)
    : NodeModel(std::move(strand)),
      _manager(std::move(manager)),
      _storage(_strand->storage().get_bucket("Clients")){};

void BrokerClientsRoot::initialize() {
  NodeModel::initialize();

  _storage->read_all([ this, keepref = get_ref() ](
                         const string_& key, std::vector<uint8_t> data,
                         BucketReadStatus read_status) mutable {
    if (PathData::invalid_name(key)) {
      // TODO check dsid
      return;
    }
    Var map =
        Var::from_json(reinterpret_cast<const char*>(data.data()), data.size());

    if (map.is_map()) {
      // TODO, remove this dispatch, as well as the above mutable, just a work
      // around before we have strand bucket
      _strand->dispatch([
        this, keepref = std::move(keepref), key, map = std::move(map)
      ]() mutable {
        // add a child dslink node
        auto child = make_ref_<BrokerClientNode>(
            _strand->get_ref(), get_ref(),
            _strand->stream_acceptor().get_profile("Broker/Client", true), key);
        child->load(map.get_map());

        add_list_child(key, child->get_ref());

        // initialize the session and responder node
        if (!child->get_client_info().responder_path.empty()) {
          _strand->session_manager().get_session(
              child->get_client_info().id, "",
              !child->get_client_info().responder_path.empty(),
              [](const ref_<Session>& session, const ClientInfo& info) {
                // do nothing
              });
        }
      });
    }
  },
                     [manager = _manager]() { manager->rebuild_path2id(); });
}

BrokerClientNode::BrokerClientNode(LinkStrandRef&& strand,
                                   ref_<BrokerClientsRoot>&& parent,
                                   ref_<NodeModel>&& profile,
                                   const string_& dsid)
    : NodeModel(std::move(strand), std::move(profile)),
      _parent(std::move(parent)),
      _client_info(dsid, "") {
  // initialize children value nodes;
  _group_node.reset(new ValueNodeModel(
      _strand->get_ref(), [ this, keepref = get_ref() ](const Var& v) {
        if (v.is_string()) {
          _client_info.group = v.get_string();
          save(*_parent->_storage, _client_info.id, false, true);
          return true;
        }
        return false;
      },
      PermissionLevel::CONFIG));
  _group_node->update_property("$type", Var("string"));
  add_list_child("Group", _group_node->get_ref());

  _path_node.reset(new ValueNodeModel(
      _strand->get_ref(), [ this, keepref = get_ref() ](const Var& v) {
        if (v.is_string()) {
          if (_client_info.max_session > 1) {
            return false;
          }
          const string_& new_path = v.get_string();
          if (new_path == _client_info.responder_path) {
            // no need to change
            return true;
          }
          string_ error =
              _parent->_manager->update_client_path(_client_info.id, new_path);
          if (error.empty()) {
            _client_info.responder_path = new_path;
            save(*_parent->_storage, _client_info.id, false, true);
            return true;
          }
        }
        return false;
      },
      PermissionLevel::CONFIG));
  _path_node->update_property("$type", Var("string"));
  add_list_child("Path", _path_node->get_ref());

  _default_token_node.reset(new ValueNodeModel(
      _strand->get_ref(), [ this, keepref = get_ref() ](const Var& v) {
        if (v.is_string()) {
          _client_info.default_token = v.get_string();
          save(*_parent->_storage, _client_info.id, false, true);
          return true;
        }
        return false;
      },
      PermissionLevel::CONFIG));
  _default_token_node->update_property("$type", Var("string"));
  add_list_child("Default_Token", _default_token_node->get_ref());

  _max_session_node.reset(new NodeModel(_strand->get_ref()));
  _max_session_node->update_property("$type", Var("number"));
  add_list_child("Max_Session", _max_session_node->get_ref());

  _current_session_node.reset(new NodeModel(_strand->get_ref()));
  _current_session_node->update_property("$type", Var("number"));
  add_list_child("Current_Session", _current_session_node->get_ref());
}
BrokerClientNode::~BrokerClientNode() = default;

void BrokerClientNode::destroy_impl() {
  _parent.reset();

  _group_node.reset();
  _path_node.reset();
  _max_session_node.reset();
  _current_session_node.reset();
  _default_token_node.reset();

  NodeModel::destroy_impl();
}
void BrokerClientNode::set_client_info(ClientInfo&& info) {
  _client_info = std::move(info);
  _group_node->set_value(Var(_client_info.group));
  _path_node->set_value(Var(_client_info.responder_path));
  _max_session_node->set_value(Var(_client_info.max_session));
  _default_token_node->set_value(Var(_client_info.default_token));
}

void BrokerClientNode::save_extra(VarMap& map) const {
  // TODO, change these to writable children value nodes
  map["?group"] = _client_info.group;
  map["?default-token"] = _client_info.default_token;
  map["?path"] = _client_info.responder_path;
  map["?max-session"] = static_cast<int64_t>(_client_info.max_session);
}
void BrokerClientNode::load_extra(VarMap& map) {
  ClientInfo info(std::move(_client_info));
  info.group = map["?group"].to_string();
  info.default_token = map["?default-token"].to_string();
  info.responder_path = map["?path"].to_string();
  info.max_session = static_cast<size_t>(map["?max-session"].get_int());
  set_client_info(std::move(info));
}

}  // namespace dsa

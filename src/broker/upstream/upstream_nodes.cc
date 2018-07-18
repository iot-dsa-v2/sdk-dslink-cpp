#include "dsa_common.h"

#include "upstream_nodes.h"

#include "../remote_node/broker_session_manager.h"
#include "../remote_node/remote_root_node.h"
#include "core/client.h"
#include "module/logger.h"
#include "module/stream_acceptor.h"
#include "network/connection.h"
#include "responder/invoke_node_model.h"
#include "responder/node_state.h"
#include "responder/value_node_model.h"

namespace dsa {

UpstreamRootNode::UpstreamRootNode(const LinkStrandRef &strand)
    : NodeModel(strand),
      _storage(_strand->storage().get_strand_bucket("upstreams", _strand)) {
  add_list_child("add",
                 make_ref_<SimpleInvokeNode>(
                     _strand, [ this, keepref = get_ref() ](Var && v)->Var {
                       if (v.is_map()) {
                         string_ node_name = v["Node Name"].to_string();
                         string_ conn_name = v["Connection Name"].to_string();
                         string_ url = v["Url"].to_string();
                         string_ token = v["Token"].to_string();
                         string_ role = v["Role"].to_string();
                         if (node_name.empty() || url.empty()) {
                           return Var(Status::INVALID_PARAMETER);
                         }
                         if (get_list_children().count(node_name) > 0) {
                           return Var(Status::INVALID_PARAMETER,
                                      "Name is already in use");
                         }
                         ref_<UpstreamConnectionNode> child =
                             make_ref_<UpstreamConnectionNode>(
                                 _strand, get_ref(),
                                 _strand->stream_acceptor().get_profile(
                                     "broker/upstream-connection", true));
                         child->_enabled = true;
                         child->_connection_name = conn_name;
                         child->_url = url;
                         child->_token = token;
                         child->_role = role;
                         add_list_child(node_name, child->get_ref());
                         child->save_upstream();
                         child->connection_changed();
                         return Var(Status::OK);
                       }
                       return Var(Status::INVALID_PARAMETER);
                     }));
}

void UpstreamRootNode::destroy_impl() {
  _storage.reset();
  NodeModel::destroy_impl();
}

void UpstreamRootNode::initialize() {
  NodeModel::initialize();

  _storage->read_all(  //
      [ this, keepref = get_ref() ](const string_ &key,
                                    std::vector<uint8_t> data,
                                    BucketReadStatus read_status) mutable {
        if (PathData::invalid_name(key)) {
          return;
        }
        Var map = Var::from_json(reinterpret_cast<const char *>(data.data()),
                                 data.size());

        if (map.is_map()) {
          // add a upstream connection

          ref_<UpstreamConnectionNode> child =
              make_ref_<UpstreamConnectionNode>(
                  _strand, get_ref(),
                  _strand->stream_acceptor().get_profile(
                      "broker/upstream-connection", true));

          child->load(map.get_map());
          add_list_child(key, child->get_ref());
        }
      },
      []() {});
}

UpstreamConnectionNode::UpstreamConnectionNode(const LinkStrandRef &strand,
                                               ref_<UpstreamRootNode> &&parent,
                                               ref_<NodeModel> &&profile)
    : NodeModel(strand, std::move(profile)), _parent(std::move(parent)) {
  _enabled_node.reset(new ValueNodeModel(
      _strand, "string",
      [ this, keepref = get_ref() ](const Var &v)->StatusDetail {
        if (v.is_bool()) {
          auto b = v.get_bool();
          if (_enabled != b) {
            _enabled = b;
            save_upstream();
            connection_changed();
          }
          return Status::DONE;
        }
        return Status::INVALID_PARAMETER;
      },
      PermissionLevel::CONFIG));
  add_list_child("enabled", _enabled_node->get_ref());

  _url_node.reset(new ValueNodeModel(
      _strand, "string",
      [ this, keepref = get_ref() ](const Var &v)->StatusDetail {
        if (v.is_string()) {
          auto str = v.get_string();
          if (str.empty()) {
            return Status::INVALID_PARAMETER;
          }
          if (_url != str) {
            _url = str;
            save_upstream();
          }
          return Status::DONE;
        }
        return Status::INVALID_PARAMETER;
      },
      PermissionLevel::CONFIG));
  add_list_child("url", _url_node->get_ref());

  _token_node.reset(new ValueNodeModel(
      _strand, "string",
      [ this, keepref = get_ref() ](const Var &v)->StatusDetail {
        if (v.is_string()) {
          auto str = v.get_string();
          if (!str.empty() && PathData::invalid_name(str)) {
            return Status::INVALID_PARAMETER;
          }
          if (_token != str) {
            _token = str;
            save_upstream();
          }
          return Status::DONE;
        }
        return Status::INVALID_PARAMETER;
      },
      PermissionLevel::CONFIG));
  add_list_child("token", _token_node->get_ref());

  _role_node.reset(new ValueNodeModel(
      _strand, "string",
      [ this, keepref = get_ref() ](const Var &v)->StatusDetail {
        if (v.is_string()) {
          auto str = v.get_string();
          if (!str.empty() && PathData::invalid_name(str)) {
            return Status::INVALID_PARAMETER;
          }
          if (_role != str) {
            _role = str;
            save_upstream();
          }
          return Status::DONE;
        }
        return Status::INVALID_PARAMETER;
      },
      PermissionLevel::CONFIG));
  add_list_child("role", _role_node->get_ref());

  _status_node.reset(new NodeModel(_strand));
  add_list_child("status", _status_node->get_ref());

  _remote_id_node.reset(new NodeModel(_strand));
  add_list_child("remote-id", _remote_id_node->get_ref());

  _remote_path_node.reset(new NodeModel(_strand));
  add_list_child("remote-path", _remote_path_node->get_ref());
}
UpstreamConnectionNode::~UpstreamConnectionNode() = default;

void UpstreamConnectionNode::update_node_values() {
  _enabled_node->set_value_lite(Var(_enabled));
  _url_node->set_value_lite(Var(_url));
  _token_node->set_value_lite(Var(_token));
  _role_node->set_value_lite(Var(_role));
}

void UpstreamConnectionNode::destroy_impl() {
  if (_client != nullptr) {
    if (_responder_node != nullptr && !_responder_node->is_destroyed()) {
      static_cast<BrokerSessionManager &>(_strand->session_manager())
          .update_responder_root(
              _responder_node->get_dsid(),
              _responder_node->get_state()->get_path().full_str(), "");
      _responder_node = nullptr;
    }
    _client->destroy();
    _client.reset();
  }
  _parent.reset();

  _enabled_node.reset();
  _url_node.reset();
  _token_node.reset();
  _role_node.reset();

  NodeModel::destroy_impl();
}

void UpstreamConnectionNode::save_upstream() const {
  save(*_parent->_storage, _state->get_path().node_name(), false, true);
}
void UpstreamConnectionNode::save_extra(VarMap &map) const {
  map[":enabled"] = _enabled;
  map[":url"] = _url;
  map[":connection-name"] = _connection_name;
  if (!_token.empty()) {
    map[":token"] = _token;
  }
  if (!_role.empty()) {
    map[":role"] = _role;
  }
}
void UpstreamConnectionNode::load_extra(VarMap &map) {
  _enabled = map[":enabled"].to_bool();
  _url = map[":url"].to_string();
  _connection_name = map[":connection-name"].to_string();
  _token = map[":token"].to_string();
  _role = map[":role"].to_string();
  update_node_values();
  connection_changed();
}

void UpstreamConnectionNode::connection_changed() {
  if (_client != nullptr) {
    if (_responder_node != nullptr && !_responder_node->is_destroyed()) {
      static_cast<BrokerSessionManager &>(_strand->session_manager())
          .update_responder_root(
              _responder_node->get_dsid(),
              _responder_node->get_state()->get_path().full_str(), "");
      _responder_node = nullptr;
    }
    _client->destroy();
    _client = nullptr;
  }
  if (_enabled) {
    WrapperStrand strand;
    strand.dsid_prefix = _connection_name;
    strand.role = _role;
    strand.client_token = _token;
    strand.strand = _strand;
    if (strand.parse_url(_url)) {
      strand.set_client_connection_maker();
      _client = make_ref_<Client>(strand);

      _client->connect([ this, keepref = get_ref() ](
                           const shared_ptr_<Connection> &connection) {
        if (is_destroyed()) {
          return;
        }
        if (connection == nullptr) {
          _status_node->set_value(Var("ReConnecting"));
        } else {
          if (_responder_node == nullptr) {
            _responder_node =
                static_cast<BrokerSessionManager &>(_strand->session_manager())
                    .add_responder_root(
                        connection->get_remote_dsid(),
                        "upstream/" + _state->get_path().node_name(),
                        _client->get_session());

            _remote_id_node->set_value(Var(connection->get_remote_dsid()));
            _remote_path_node->set_value(Var(connection->get_remote_path()));
          }
          _status_node->set_value(Var("Connected"));
        }

      },
                       Client::EVERY_CONNECTION);
    }
    _status_node->set_value(Var("Connecting"));
    _remote_id_node->set_value(Var());
    _remote_path_node->set_value(Var());
  } else {
    _status_node->set_value(Var("Disconnected"));
    _remote_id_node->set_value(Var());
    _remote_path_node->set_value(Var());
  }
}
}  // namespace dsa

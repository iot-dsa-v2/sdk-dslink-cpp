#include "dsa_common.h"

#include "token_nodes.h"

#include "broker_client_manager.h"
#include "core/strand_timer.h"
#include "module/storage.h"
#include "module/stream_acceptor.h"
#include "responder/invoke_node_model.h"
#include "responder/node_state.h"
#include "responder/value_node_model.h"
#include "util/date_time.h"
#include "util/string.h"

namespace dsa {
TokensRoot::TokensRoot(const LinkStrandRef &strand,
                       ref_<BrokerClientManager> &&manager)
    : NodeModel(strand),
      _manager(std::move(manager)),
      _storage(_strand->storage().get_strand_bucket("tokens", _strand)) {
  add_list_child(
      "add",
      make_ref_<SimpleInvokeNode>(
          strand, [ this, keepref = get_ref() ](Var && v)->Var {
            if (v.is_map()) {
              string_ role = v["Role"].to_string();
              string_ time_range = v["Time_Range"].to_string();
              int64_t count = v["Count"].to_int(-1);
              int64_t max_session = v["Max_Session"].to_int(1);
              bool managed = v["Managed"].to_bool();

              string_ token_name = generate_random_string(16);
              while (_list_children.count(token_name) > 0) {
                token_name = generate_random_string(16);
              }
              string_ token = token_name + generate_random_string(32);

              auto new_token = make_ref_<TokenNode>(
                  _strand, get_ref(),
                  _strand->stream_acceptor().get_profile("broker/token", true),
                  token, role, time_range, count, max_session, managed);
              add_list_child(token_name, new_token->get_ref());
              new_token->save_token();
              return Var({{"Token", Var(token)}});
            }
            return Var(Status::INVALID_PARAMETER);
          },
          PermissionLevel::CONFIG));
}
TokensRoot::~TokensRoot() = default;

void TokensRoot::destroy_impl() {
  NodeModel::destroy_impl();
  _storage.reset();
  _manager.reset();
}

void TokensRoot::initialize() {
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
          // add a child dslink node
          ref_<TokenNode> child;
          child = make_ref_<TokenNode>(
              _strand, get_ref(),
              _strand->stream_acceptor().get_profile("broker/token", true));
          child->load(map.get_map());

          add_list_child(key, child->get_ref());
        }
      },
      []() {});
}

TokenNode::TokenNode(const LinkStrandRef &strand, ref_<TokensRoot> &&parent,
                     ref_<NodeModel> &&profile, const string_ &token,
                     const string_ &role, const string_ &time_range,
                     int64_t count, int64_t max_session, bool managed)
    : NodeModel(strand, std::move(profile)),
      _parent(std::move(parent)),
      _token(token),
      _role(role),
      _time_range(time_range),
      _count(count),
      _max_session(max_session),
      _managed(managed) {
  _token_node.reset(new NodeModel(_strand));
  _token_node->update_property("$type", Var("string"));
  add_list_child("token", _token_node->get_ref());

  _role_node.reset(new ValueNodeModel(
      _strand, "string",
      [ this, keepref = get_ref() ](const Var &v)->StatusDetail {
        if (v.is_string()) {
          auto &str = v.get_string();
          if (PathData::invalid_name(str)) {
            return Status::INVALID_PARAMETER;
          }
          if (_role != str) {
            _role = str;
            save_token();
          }
          return Status::DONE;
        }
        return Status::INVALID_PARAMETER;
      },
      PermissionLevel::CONFIG));
  add_list_child("role", _role_node->get_ref());

  _time_range_node.reset(new ValueNodeModel(
      _strand, "string",
      [ this, keepref = get_ref() ](const Var &v)->StatusDetail {
        if (v.is_string()) {
          auto &str = v.get_string();
          if (str != _time_range) {
            if (udpate_timerange(str)) {
              save_token();
            } else {
              return StatusDetail(Status::INVALID_PARAMETER,
                                  "TimeRange format error");
            }
          }
          return Status::DONE;
        }
        return Status::INVALID_PARAMETER;
      },
      PermissionLevel::CONFIG));
  add_list_child("time-range", _time_range_node->get_ref());

  _count_node.reset(new ValueNodeModel(
      _strand, "number",
      [ this, keepref = get_ref() ](const Var &v)->StatusDetail {
        if (v.is_int()) {
          int64_t n = v.get_int();
          if (_count != n) {
            _count = n;
            save_token();
          }
          return Status::DONE;
        }
        return Status::INVALID_PARAMETER;
      },
      PermissionLevel::CONFIG));
  add_list_child("count", _count_node->get_ref());

  _max_session_node.reset(new ValueNodeModel(
      _strand, "number",
      [ this, keepref = get_ref() ](const Var &v)->StatusDetail {
        if (v.is_int()) {
          int64_t n = v.get_int();
          if (_count != n) {
            _count = n;
            save_token();
          }
          return Status::DONE;
        }
        return Status::INVALID_PARAMETER;
      },
      PermissionLevel::CONFIG));
  add_list_child("max-session", _max_session_node->get_ref());

  _managed_node.reset(new ValueNodeModel(
      _strand, "bool",
      [ this, keepref = get_ref() ](const Var &v)->StatusDetail {
        if (v.is_bool()) {
          bool b = v.get_bool();
          if (_managed != b) {
            _managed = b;
            save_token();
          }
          return Status::DONE;
        }
        return Status::INVALID_PARAMETER;
      },
      PermissionLevel::CONFIG));
  add_list_child("managed", _managed_node->get_ref());

  update_node_values();
}
TokenNode::~TokenNode() = default;

void TokenNode::destroy_impl() {
  if (_timer != nullptr) {
    _timer->destroy();
    _timer.reset();
  }
  _parent.reset();
  _token_node.reset();
  _role_node.reset();
  _time_range_node.reset();
  _count_node.reset();
  _max_session_node.reset();
  _managed_node.reset();

  NodeModel::destroy_impl();
}

void TokenNode::update_node_values() {
  _token_node->set_value_lite(Var(_token));
  _role_node->set_value_lite(Var(_role));
  _time_range_node->set_value_lite(Var(_time_range));
  _count_node->set_value_lite(Var(_count));
  _max_session_node->set_value_lite(Var(_role));
  _managed_node->set_value_lite(Var(_managed));
}

bool TokenNode::udpate_timerange(const string_ &value) {
  if (!value.empty()) {
    auto slash_pos = _time_range.find('/');
    if (slash_pos != string_::npos) {
      _valid_from = DateTime::parse_ts(_time_range.substr(0, slash_pos));
      _valid_to = DateTime::parse_ts(_time_range.substr(slash_pos + 1));
      if (_valid_from != LLONG_MIN && _valid_to != LLONG_MIN) {
        _time_range = value;

        if (_timer != nullptr) {
          _timer->destroy();
          _timer = nullptr;
        }
        auto ts = DateTime::ms_since_epoch();
        if (ts < _valid_to) {
          _timer = _strand->add_timer(
              _valid_to - ts, [ this, keepref = get_ref() ](bool canceled) {
                if (canceled) return false;
                if (_managed) {
                  remove_all_clients();
                }
                return false;
              });
        } else
          return true;
      }
    }
    return false;
  }

  _time_range = value;
  if (_timer != nullptr) {
    _timer->destroy();
    _timer = nullptr;
  }
  _valid_from = LLONG_MIN;
  _valid_to = LLONG_MIN;
  return true;
}

bool TokenNode::is_valid() {
  if (_count == 0) return false;
  if (_valid_from != LLONG_MIN) {
    auto ts = DateTime::ms_since_epoch();
    return ts >= _valid_from && ts < _valid_to;
  }

  return true;
}

void TokenNode::remove_all_clients() {
  this->_parent->_manager->remove_clients_from_token(
      _state->get_path().node_name());
}

void TokenNode::regenerate() {
  // only change the rest
  _token = _token.substr(0, 16) + generate_random_string(32);
  _token_node->set_value(Var(_token));
  save_token();
}

void TokenNode::save_token() const {
  save(*_parent->_storage, _state->get_path().node_name(), false, true);
}

void TokenNode::save_extra(VarMap &map) const {
  map[":token"] = _token;
  map[":role"] = _role;
  if (!_time_range.empty()) {
    map[":time-range"] = _time_range;
  }
  if (_count >= 0) {
    map[":count"] = _count;
  }
  if (_max_session > 1) {
    map[":max-session"] = _max_session;
  }
  if (_managed) {
    map[":managed"] = true;
  }
}
void TokenNode::load_extra(VarMap &map) {
  _token = map[":token"].to_string();
  _role = map[":role"].to_string();
  _time_range = map[":time-range"].to_string();
  _count = map[":count"].to_int(-1);
  _max_session = map[":max-session"].to_int(1);
  _managed = map[":managed"].to_bool();
  update_node_values();
}
}  // namespace dsa

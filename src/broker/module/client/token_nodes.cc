#include "dsa_common.h"

#include "token_nodes.h"

#include "module/storage.h"
#include "module/stream_acceptor.h"
#include "responder/invoke_node_model.h"
#include "responder/node_state.h"
#include "responder/value_node_model.h"
#include "util/string.h"

namespace dsa {
TokensRoot::TokensRoot(const LinkStrandRef &strand)
    : NodeModel(strand),
      _storage(_strand->storage().get_strand_bucket("Tokens", _strand)) {
  add_list_child(
      "Add",
      make_ref_<SimpleInvokeNode>(
          strand, [ this, keepref = get_ref() ](Var && v)->Var {
            if (v.is_map()) {
              string_ role = v["Role"].to_string();
              string_ time_range = v["Time_Range"].to_string();
              int64_t count = v["Count"].to_int(-1);
              int64_t max_session = v["Max_Session"].to_int(1);
              bool managed = v["Manager"].to_bool();

              string_ token_name = generate_random_string(16);
              while (_list_children.count(token_name) > 0) {
                token_name = generate_random_string(16);
              }
              string_ token = token_name + generate_random_string(32);

              auto new_token = make_ref_<TokenNode>(
                  _strand, get_ref(),
                  _strand->stream_acceptor().get_profile("Broker/Token", true),
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
  _token_node->set_value_lite(Var(_token));
  _token_node->update_property("$type", Var("string"));
  add_list_child("Token", _token_node->get_ref());

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
  _role_node->set_value_lite(Var(_role));
  add_list_child("Role", _role_node->get_ref());

  _time_range_node.reset(new ValueNodeModel(
      _strand, "string",
      [ this, keepref = get_ref() ](const Var &v)->StatusDetail {
        if (v.is_string()) {
          auto &str = v.get_string();
          if (str != _time_range) {
            _time_range = str;
            save_token();
            udpate_timerange();
          }
          return Status::DONE;
        }
        return Status::INVALID_PARAMETER;
      },
      PermissionLevel::CONFIG));
  _time_range_node->set_value_lite(Var(_time_range));
  add_list_child("Time_Range", _time_range_node->get_ref());

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
  _count_node->set_value_lite(Var(_count));
  add_list_child("Count", _count_node->get_ref());

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
  _max_session_node->set_value_lite(Var(_role));
  add_list_child("Max_Session", _max_session_node->get_ref());

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
  _managed_node->set_value_lite(Var(_managed));
  add_list_child("Managed", _managed_node->get_ref());
}
TokenNode::~TokenNode() = default;

void TokenNode::udpate_timerange() {
  // TODO
}

bool TokenNode::is_valid() {
  if (_count == 0) return false;
  //TODO check time range
  return true;

}

void TokenNode::remove_all_clients() {
  // TODO
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

}  // namespace dsa
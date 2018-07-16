#include "dsa_common.h"

#include "permission_nodes.h"

#include "module/logger.h"
#include "module/stream_acceptor.h"
#include "responder/node_state.h"
#include "responder/value_node_model.h"
#include "util/string.h"
#include "util/string_encode.h"

namespace dsa {

static const string_ DEFAULT_ROLE_NAME = "default";

PermissionRoleRootNode::PermissionRoleRootNode(const LinkStrandRef &strand)
    : NodeModel(strand),
      _storage(_strand->storage().get_strand_bucket("roles", _strand)) {}

void PermissionRoleRootNode::destroy_impl() {
  _storage.reset();
  NodeModel::destroy_impl();
}

void PermissionRoleRootNode::initialize() {
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
          ref_<PermissionRoleNode> child;
          if (key == DEFAULT_ROLE_NAME) {
            child = make_ref_<PermissionRoleNode>(_strand, get_ref(), nullptr);
          } else {
            child = make_ref_<PermissionRoleNode>(
                _strand, get_ref(),
                _strand->stream_acceptor().get_profile("broker/permission-role",
                                                       true));
          }
          child->load(map.get_map());

          add_list_child(key, child->get_ref());
        }
      },
      [ this, keepref = get_ref() ]() {
        if (_list_children.count(DEFAULT_ROLE_NAME) > 0) {
          // check existing default role
          _default_role.reset(dynamic_cast<PermissionRoleNode *>(
              _list_children[DEFAULT_ROLE_NAME].get()));
        }
        if (_default_role == nullptr) {
          _default_role =
              make_ref_<PermissionRoleNode>(_strand, get_ref(), nullptr);
          _default_role->set_value_lite(Var(PermissionName::CONFIG));
          _default_role->_default_level = PermissionLevel ::CONFIG;
          add_list_child(DEFAULT_ROLE_NAME, _default_role->get_ref());
        }
        init_default_role_node();
      });
}

// default role node doesn't use the Permission_Role profile, and needs to be
// handled separately
void PermissionRoleRootNode::init_default_role_node() {
  if (_default_role == nullptr) {
    return;
  }
  _default_role->update_property("$type", Var("string"));
  _default_role->update_property("$editor",
                                 Var("enum[,none,list,read,write,config]"));
  if (_default_role->_default_level == PermissionLevel::INVALID) {
    _default_role->_default_level = PermissionLevel::NONE;
    _default_role->set_value_lite(Var(PermissionName::NONE));
  }
  // copy a profile node into list children
  // since the profile node is already in state tree
  // its state will still be the original one under Pub node
  _default_role->add_list_child("add-rule",
                                _strand->stream_acceptor().get_profile(
                                    "broker/permission-role/add-rule", true));

  // remove fallback node
  _default_role->remove_list_child("fallback");
}

PermissionRoleNode::PermissionRoleNode(const LinkStrandRef &strand,
                                       ref_<PermissionRoleRootNode> &&parent,
                                       ref_<NodeModel> &&profile)
    : NodeModel(strand, std::move(profile), PermissionLevel::CONFIG),
      _parent(std::move(parent)) {
  _fallback_name_node.reset(new ValueNodeModel(
      _strand, "string",
      [ this, keepref = get_ref() ](const Var &v)->StatusDetail {
        if (v.is_string()) {
          if (v.get_string() != _fallback_name) {
            _fallback_name = _fallback_name;
            _fallback_role_cache.reset();
            save_role();
          }

          return Status::DONE;
        }
        return Status::INVALID_PARAMETER;
      }));
  add_list_child("fallback", std::move(_fallback_name_node));
}
PermissionRoleNode::~PermissionRoleNode() = default;

PermissionLevel PermissionRoleNode::get_permission(const string_ &path,
                                                   int loop_protect) {
  for (auto it = _rules.rbegin(); it != _rules.rend(); ++it) {
    if (str_starts_with(path, it->first)) {
      // math full path or match as folder
      if (path.size() == it->first.size() || path[it->first.size()] == '/') {
        return it->second;
      }
    }
  }
  if (_default_level != PermissionLevel::INVALID) {
    return _default_level;
  }
  if (loop_protect <= 0) {
    LOG_ERROR(
        __FILENAME__,
        LOG << "find loop in permission role fallback chain, ends at role: "
            << _fallback_name);
    return PermissionLevel ::NONE;
  }
  return get_fallback_role()->get_permission(path, loop_protect - 1);
}
ref_<PermissionRoleNode> &PermissionRoleNode::get_fallback_role() {
  if (_fallback_role_cache == nullptr || _fallback_role_cache->is_destroyed()) {
    _fallback_role_cache = nullptr;
    auto role = _parent->get_child(_fallback_name);
    if (role != nullptr) {
      _fallback_role_cache.reset(
          dynamic_cast<PermissionRoleNode *>(role.get()));
    }
    if (_fallback_role_cache == nullptr) {
      _fallback_role_cache = _parent->_default_role;
    }

    if (_fallback_role_cache == this) {
      LOG_ERROR(__FILENAME__,
                LOG << "reference self as fallback role: " << _fallback_name);
    }
  }

  return _fallback_role_cache;
}
void PermissionRoleNode::destroy_impl() {
  _parent.reset();
  _fallback_name_node.reset();
  _fallback_role_cache.reset();

  NodeModel::destroy_impl();
}
StatusDetail PermissionRoleNode::on_set_value(MessageValue &&value) {
  if (value.value.is_string()) {
    const string_ &str = value.value.get_string();
    auto level = PermissionName::parse(str);

    if (level != PermissionLevel::INVALID || str.empty()) {
      if (level != _default_level) {
        _default_level = level;
        auto result = NodeModel::on_set_value(std::move(value));
        save(*_parent->_storage, _state->get_path().node_name(), false, true);
        return std::move(result);
      }
    }
  }
  return Status::INVALID_PARAMETER;
}
void PermissionRoleNode::save_role() const {
  save(*_parent->_storage, _state->get_path().node_name(), false, true);
}
void PermissionRoleNode::save_extra(VarMap &map) const {
  if (!_fallback_name.empty()) {
    map[":fallback"] = _fallback_name;
  }

  auto rules = make_ref_<VarMap>();
  for (auto it : _rules) {
    (*rules)[it.first] = Var(PermissionName::convert(it.second));
  }
  map[":rules"] = std::move(rules);
}
void PermissionRoleNode::load_extra(VarMap &map) {
  if (_cached_value != nullptr) {
    _default_level =
        PermissionName::parse(_cached_value->get_value().value.to_string());
  }

  if (map.count(":fallback") > 0 && map[":fallback"].is_string()) {
    _fallback_name = map[":fallback"].get_string();
  } else {
    _fallback_name = "";
  }

  if (map.count(":rules") > 0 && map[":rules"].is_map()) {
    auto &rules = map[":rules"].get_map();
    for (auto &it : rules) {
      if (it.second.is_string()) {
        auto &str = it.second.get_string();
        auto level = PermissionName::parse(str);
        if (level != PermissionLevel::INVALID) {
          _rules[it.first] = level;

          // create a rule node for it
          auto rule = make_ref_<PermissionRuleNode>(
              _strand, get_ref(),
              _strand->stream_acceptor().get_profile("broker/permission-rule",
                                                     true));
          rule->_path = it.first;
          rule->_level = level;
          rule->set_value_lite(Var(str));

          add_list_child(url_encode_node_name(it.first),
                         ref_<NodeModelBase>(rule));
        } else {
          LOG_ERROR(__FILENAME__,
                    LOG << "find invalid permission rule during loading: "
                        << it.first << " : " << str);
        }
      }
    }
  }
}

PermissionRuleNode::PermissionRuleNode(const LinkStrandRef &strand,
                                       ref_<PermissionRoleNode> &&role,
                                       ref_<NodeModel> &&profile)
    : NodeModel(strand, std::move(profile)), _role(std::move(role)) {}

void PermissionRuleNode::destroy_impl() {
  _role.reset();
  NodeModel::destroy_impl();
}
StatusDetail PermissionRuleNode::on_set_value(MessageValue &&value) {
  if (value.value.is_string()) {
    auto level = PermissionName::parse(value.value.get_string());
    if (level != PermissionLevel::INVALID) {
      if (level != _level) {
        _level = level;
        _role->_rules[_path] = _level;
        _role->save_role();
        return NodeModel::on_set_value(std::move(value));
      }
    }
  }
  return Status::INVALID_PARAMETER;
}
}  // namespace dsa

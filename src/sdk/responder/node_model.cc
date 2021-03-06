#include "dsa_common.h"

#include "node_model.h"

#include "message/request/set_request_message.h"
#include "message/response/set_response_message.h"
#include "module/logger.h"
#include "module/storage.h"
#include "node_state.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_set_stream.h"
#include "util/string_encode.h"
namespace dsa {

static const std::vector<string_> default_summary_metas = {
    "$is", "$type", "$writable", "$invokable"};

NodeModel::NodeModel(const LinkStrandRef &strand,
                     PermissionLevel write_require_permission)
    : NodeModelBase(strand) {
  set_value_require_permission(write_require_permission);
};
NodeModel::NodeModel(const LinkStrandRef &strand, ref_<NodeModel> &&profile,
                     PermissionLevel write_require_permission)
    : NodeModelBase(strand), _profile(std::move(profile)) {
  set_value_require_permission(write_require_permission);

  if (_profile != nullptr) {
    auto &state = _profile->get_state();
    if (state == nullptr || state->get_path().data()->names[0] != "pub") {
      LOG_FATAL(__FILENAME__, LOG << "invalid profile node");
    }

    update_property("$is", Var(state->get_path().move_pos(1).remain_str()));

    // copy necessary data for the permission and summary
    if (_profile->_metas.count("$type") > 0) {
      _metas["$type"] = _profile->_metas["$type"];
    }
    if (_set_value_require_permission == PermissionLevel::INVALID &&
        _profile->_metas.count("$writable") > 0) {
      // overwrite the permission when it's not defined in constructor
      _metas["$writable"] = _profile->_metas["$writable"];
      _set_value_require_permission =
          PermissionName::parse(_metas["$writable"]->get_value().to_string());
    }
    if (_profile->_metas.count("$invokable") > 0) {
      _metas["$invokable"] = _profile->_metas["$invokable"];
    }
  }
}

void NodeModel::set_value_require_permission(PermissionLevel permission_level) {
  if (permission_level >= PermissionLevel::WRITE &&
      permission_level <= PermissionLevel::CONFIG) {
    _set_value_require_permission = permission_level;
    update_property("$writable",
                    Var(PermissionName::convert((permission_level))));
  }
}

VarBytesRef NodeModel::get_summary() {
  if (_summary == nullptr) {
    Var v = Var::new_map();
    VarMap &map = v.get_map();

    for (auto &key : default_summary_metas) {
      auto find = _metas.find(key);
      if (find != _metas.end()) {
        map[key] = find->second->get_value();
      }
    }

    _summary.reset(new VarBytes(std::move(v)));
  }
  return _summary;
}

void NodeModel::destroy_impl() {
  // clear children just in case some children keep parent's ref
  _list_children.clear();
  _metas.clear();
  _attributes.clear();
  _summary.reset();
  NodeModelBase::destroy_impl();
}

void NodeModel::initialize() {
  for (auto &it : _list_children) {
    add_child(it.first, ModelRef(it.second.get()));
  }
  if (_profile != nullptr) {
    for (auto &it : _profile->_list_children) {
      auto child_state = _state->get_child(it.first, true);
      // add profile node only when it doesn't exist already
      if (child_state->get_model() == nullptr) {
        child_state->set_model(it.second->get_ref());
      } else {
        // remove profile node in list children
        _list_children.erase(it.first);
      }
    }
  }
}

void NodeModel::on_list(BaseOutgoingListStream &stream, bool first_request) {
  if (_profile != nullptr) {
    stream.update_list_pub_path("pub");
  }
  send_props_list(stream);
  send_children_list(stream);
  stream.update_response_status(Status::OK);
}

void NodeModel::on_subscribe(const SubscribeOptions &options,
                             bool first_request) {
  if (first_request && _cached_value == nullptr && _metas.count("$type") == 0) {
    auto response = make_ref_<SubscribeResponseMessage>();
    response->set_status(Status::NOT_SUPPORTED);
    set_subscribe_response(std::move(response));
  }
}

void NodeModel::send_props_list(BaseOutgoingListStream &stream) {
  for (auto &it : _metas) {
    stream.update_list_value(it.first, it.second);
  }
  for (auto &it : _attributes) {
    stream.update_list_value(it.first, it.second);
  }
}
void NodeModel::send_children_list(BaseOutgoingListStream &stream) {
  for (auto &it : _list_children) {
    stream.update_list_value(it.first, it.second->get_summary());
  }
}

void NodeModel::update_property(const string_ &field, VarBytesRef &&value) {
  if (!field.empty()) {
    if (field[0] == '$') {
      _metas[field] = value;
    } else if (field[0] == '@') {
      _attributes[field] = value;
    } else {
      return;
    }
    if (_need_list && _state != nullptr) {
      _state->update_list_value(field, value);
    }
  }
}
ref_<NodeModelBase> NodeModel::add_list_child(const string_ &name,
                                              ref_<NodeModelBase> &&model) {
  _list_children[name] = model;
  if (_state != nullptr) {
    add_child(name, ModelRef(model.get()));
    if (_need_list) {
      _state->update_list_value(name, model->get_summary());
    }
  }
  return std::move(model);
}
void NodeModel::remove_list_child(const string_ &name) {
  _list_children.erase(name);
  if (_state != nullptr) {
    remove_child(name);
    _state->update_list_removed(name);
  }
}

void NodeModel::set(ref_<OutgoingSetStream> &&stream) {
  OutgoingSetStream *raw_stream_pat = stream.get();
  raw_stream_pat->on_request([
    this, ref = get_ref(), stream = std::move(stream),
    paged_cache = ref_<IncomingPageCache<SetRequestMessage>>()
  ](OutgoingSetStream & s, ref_<const SetRequestMessage> && message) mutable {
    if (message == nullptr) {
      return;  // nullptr is for destroyed callback, no need to handle here
    }
    auto field = message->get_attribute_field();
    StatusDetail status;
    if (field.empty()) {
      if (_set_value_require_permission >= PermissionLevel::INVALID) {
        status = Status::NOT_SUPPORTED;
      } else if (stream->allowed_permission < _set_value_require_permission) {
        status = Status::PERMISSION_DENIED;
      } else {
        // try merging paged group
        message = IncomingPageCache<SetRequestMessage>::get_first_page(
            paged_cache, std::move(message));
        if (message == nullptr) {
          // paged message is not ready
          return;
        }
        status = on_set_value(message->get_value());
      }

    } else {
      status = on_set_attribute(field, std::move(message->get_value().value));
    }
    auto response = make_ref_<SetResponseMessage>();
    response->set_status(status.code);
    if (!status.detail.empty()) {
      response->set_error_detail(status.detail);
    }
    s.send_response(std::move(response));
  });
}

StatusDetail NodeModel::on_set_value(MessageValue &&value) {
  set_value(std::move(value));
  return Status::DONE;
}
StatusDetail NodeModel::on_set_attribute(const string_ &field, Var &&value) {
  return Status::NOT_SUPPORTED;
}

// serialization

void NodeModel::save(StorageBucket &storage, const string_ &storage_path,
                     bool recursive, bool use_json) const {
  auto map = make_ref_<VarMap>();
  if (_cached_value != nullptr) {
    (*map)[":value"] = _cached_value->get_value().value;
    if (!_cached_value->get_value().meta.is_null()) {
      (*map)[":vmeta"] = _cached_value->get_value().meta;
    }
  }
  for (auto &it : _metas) {
    if (save_meta(it.first)) {
      (*map)[it.first] = it.second->get_value();
    }
  }
  for (auto &it : _attributes) {
    if (save_attribute(it.first)) {
      (*map)[it.first] = it.second->get_value();
    }
  }
  string_ recursive_path;
  if (recursive && !storage_path.empty()) {
    recursive_path = storage_path + '/';
  }
  for (auto &it : _list_children) {
    if (save_child(it.first)) {
      (*map)[it.first] = it.second->get_summary()->get_value();
      if (recursive) {
        // save child if it's also a NodeModel
        auto child_model = dynamic_cast<NodeModel *>(it.second.get());
        if (child_model != nullptr) {
          child_model->save(storage, recursive_path + it.first, true, use_json);
        }
      }
    }
  }
  save_extra(*map);
  Var var;
  var = map;
  if (use_json) {
    string_ str = var.to_json(1);
    const uint8_t *str_data = reinterpret_cast<const uint8_t *>(str.data());
    storage.write(storage_path,
                  make_ref_<RefCountBytes>(str_data, str_data + str.length()));
  } else {
    storage.write(storage_path, make_ref_<RefCountBytes>(var.to_msgpack()));
  }
}
void NodeModel::load(VarMap &map) {
  if (map.count(":value")) {
    MessageValue v(map[":value"]);
    if (map.count(":vmeta") && map[":vmeta"].is_map()) {
      v.meta = map[":vmeta"];
    }
    set_value(std::move(v));
  }
  for (auto &it : map) {
    if (it.first.empty()) continue;
    switch (it.first[0]) {
      case '$': {
        _metas[it.first] = make_ref_<VarBytes>(it.second);
      } break;
      case '@': {
        _attributes[it.first] = make_ref_<VarBytes>(it.second);
      } break;
      case '?':
        // already handled, do nothing
        break;
      default:
        if (it.second.is_map()) {
          on_load_child(it.first, it.second.get_map());
        }
    }
  }
  load_extra(map);
}

}  // namespace dsa

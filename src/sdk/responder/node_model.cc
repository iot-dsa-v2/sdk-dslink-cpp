#include "dsa_common.h"

#include "node_model.h"

#include "message/request/set_request_message.h"
#include "message/response/set_response_message.h"
#include "module/logger.h"
#include "node_state.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_set_stream.h"

namespace dsa {
ModelProperty::ModelProperty()
    : _bytes(new RefCountBytes()), _value_ready(false) {}
ModelProperty::ModelProperty(BytesRef &bytes)
    : _bytes(bytes), _value_ready(false) {}
ModelProperty::ModelProperty(Var &&value)
    : _value(std::move(value)), _value_ready(true) {}

BytesRef &ModelProperty::get_bytes() const {
  if (_bytes == nullptr && _value_ready) {
    _bytes.reset(new RefCountBytes(_value.to_msgpack()));
  }
  return _bytes;
}

const Var &ModelProperty::get_value() const {
  if (!_value_ready) {
#ifdef DSA_DEBUG
    if (_bytes->empty()) {
      LOG_FATAL(LOG << "invalid value in ModelProperty");
    }
#endif
    _value = Var::from_msgpack(_bytes->data(), _bytes->size());
    _value_ready = true;
  }
  return _value;
}

static const std::vector<string_> default_summary_metas = {
    "$is", "$type", "$writable", "$invokable"};

NodeModel::NodeModel(LinkStrandRef &&strand,
                     PermissionLevel write_require_permission)
    : NodeModelBase(std::move(strand)) {
  set_value_require_permission(write_require_permission);
};
void NodeModel::set_value_require_permission(PermissionLevel permission_level) {
  if (permission_level >= PermissionLevel::WRITE &&
      permission_level <= PermissionLevel::CONFIG) {
    _set_value_require_permission = permission_level;
    _metas["$writable"] = Var(to_string(permission_level));
  }
}

BytesRef &NodeModel::get_summary() {
  if (_summary == nullptr) {
    Var v = Var::new_map();
    VarMap &map = v.get_map();

    for (auto &key : default_summary_metas) {
      auto find = _metas.find(key);
      if (find != _metas.end()) {
        map[key] = find->second.get_value();
      }
    }

    _summary = new RefCountBytes(v.to_msgpack());
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
}

void NodeModel::on_list(BaseOutgoingListStream &stream, bool first_request) {
  send_props_list(stream);
  send_children_list(stream);
}

void NodeModel::send_props_list(BaseOutgoingListStream &stream) {
  for (auto &it : _metas) {
    stream.update_list_value(it.first, it.second.get_bytes());
  }
  for (auto &it : _attributes) {
    stream.update_list_value(it.first, it.second.get_bytes());
  }
}
void NodeModel::send_children_list(BaseOutgoingListStream &stream) {
  for (auto &it : _list_children) {
    stream.update_list_value(it.first, it.second->get_summary());
  }
}

void NodeModel::update_property(const string_ &field, ModelProperty &&value) {
  if (!field.empty()) {
    if (field[0] == '$') {
      _metas[field] = value;
    } else if (field[0] == '@') {
      _attributes[field] = value;
    } else {
      return;
    }
    if (_need_list && _state != nullptr) {
      _state->update_list_value(field, value.get_bytes());
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

void NodeModel::set(ref_<OutgoingSetStream> &&stream) {
  OutgoingSetStream *raw_stream_pat = stream.get();
  raw_stream_pat->on_request([
    this, ref = get_ref(), stream = std::move(stream)
  ](OutgoingSetStream & s, ref_<const SetRequestMessage> && message) {
    auto field = message->get_attribute_field();
    MessageStatus status;
    if (field.empty()) {
      if (_set_value_require_permission >= PermissionLevel::NEVER) {
        status = MessageStatus::NOT_SUPPORTED;
      } else if (stream->allowed_permission < _set_value_require_permission) {
        status = MessageStatus::PERMISSION_DENIED;
      }
      status = on_set_value(message->get_value());
    } else {
      status = on_set_attribute(field, std::move(message->get_value().value));
    }
    auto response = make_ref_<SetResponseMessage>();
    response->set_status(status);
    s.send_response(std::move(response));
  });
}

MessageStatus NodeModel::on_set_value(MessageValue &&value) {
  set_value(std::move(value));
  return MessageStatus::CLOSED;
}
MessageStatus NodeModel::on_set_attribute(const string_ &field, Var &&value) {
  return MessageStatus::NOT_SUPPORTED;
}

}  // namespace dsa

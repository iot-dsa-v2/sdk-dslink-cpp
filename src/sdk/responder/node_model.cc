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
    : _bytes(new IntrusiveBytes()), _value_ready(false) {}
ModelProperty::ModelProperty(BytesRef &bytes)
    : _bytes(bytes), _value_ready(false) {}
ModelProperty::ModelProperty(Variant &&value)
    : _value(std::move(value)), _value_ready(true) {}

BytesRef &ModelProperty::get_bytes() const {
  if (_bytes == nullptr && _value_ready) {
    _bytes.reset(new IntrusiveBytes(_value.to_msgpack()));
  }
  return _bytes;
}

const Variant &ModelProperty::get_value() const {
  if (!_value_ready) {
#ifdef DSA_DEBUG
    if (_bytes->empty()) {
      LOG_FATAL(LOG << "invalid value in ModelProperty");
    }
#endif
    _value = Variant::from_msgpack(_bytes->data(), _bytes->size());
    _value_ready = true;
  }
  return _value;
}

static const std::vector<std::string> default_summary_metas = {
    "$is", "$type", "$writable", "$invokable"};

BytesRef &NodeModel::get_summary() {
  if (_summary == nullptr) {
    Variant v = Variant::new_map();
    VariantMap &map = v.get_map();

    for (auto &key : default_summary_metas) {
      auto find = _metas.find(key);
      if (find != _metas.end()) {
        map[key] = find->second.get_value();
      }
    }

    _summary = new IntrusiveBytes(v.to_msgpack());
  }
  return _summary;
}

void NodeModel::close_impl() {
  // clear children just in case some children keep parent's ref
  _list_children.clear();
  NodeModelBase::close_impl();
}

void NodeModel::initialize() {
  for (auto &it : _list_children) {
    add_child(it.first, ModelRef(it.second.get()));
  }
}

void NodeModel::init_list_stream(OutgoingListStream &stream) {
  send_props_list(stream);
  send_children_list(stream);
}

void NodeModel::send_props_list(OutgoingListStream &stream) {
  for (auto &it : _metas) {
    stream.update_value(it.first, it.second.get_bytes());
  }
  for (auto &it : _attributes) {
    stream.update_value(it.first, it.second.get_bytes());
  }
}
void NodeModel::send_children_list(OutgoingListStream &stream) {
  for (auto &it : _list_children) {
    stream.update_value(it.first, it.second->get_summary());
  }
}

void NodeModel::update_property(const std::string &field,
                                ModelProperty &&value) {
  if (!field.empty()) {
    if (field[0] == '$') {
      _metas[field] = value;
    } else if (field[0] == '@') {
      _attributes[field] = value;
    } else {
      return;
    }
    if (_state != nullptr) {
      _state->update_list_value(field, value.get_bytes());
    }
  }
}
ref_<NodeModel> NodeModel::add_list_child(const std::string &name,
                                          ref_<NodeModel> &&model) {
  _list_children[name] = model;
  if (_state != nullptr) {
    add_child(name, ModelRef(model.get()));
    _state->update_list_value(name, model->get_summary());
  }
  return std::move(model);
}

void NodeModel::on_set(ref_<OutgoingSetStream> &&stream) {
  stream->on_request([this, ref=get_ref()](OutgoingSetStream &s,
                            ref_<const SetRequestMessage> &&message) {
    auto field = message->get_attribute_field();
    MessageStatus status;
    if (field.empty()) {
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
  if (allows_set_value()) {
    set_value(std::move(value));
    return MessageStatus::CLOSED;
  }
  return MessageStatus::NOT_SUPPORTED;
}
MessageStatus NodeModel::on_set_attribute(const std::string &field,
                                          Variant &&value) {
  return MessageStatus::NOT_SUPPORTED;
}

}  // namespace dsa

#include "dsa_common.h"

#include "node_model.h"

#include "module/logger.h"
#include "node_state.h"
#include "stream/responder/outgoing_list_stream.h"

namespace dsa {
ModelProperty::ModelProperty() : _value_ready(false) {}
ModelProperty::ModelProperty(BytesRef &bytes)
    : _bytes(bytes), _value_ready(false) {}
ModelProperty::ModelProperty(Variant &&value)
    : _value(std::move(value)), _value_ready(true) {}

BytesRef &ModelProperty::get_bytes() const {
  if (_bytes == nullptr) {
#ifdef DSA_DEBUG
    if (!_value_ready) {
      LOG_FATAL(Logger::_(), LOG << "invalid value in ModelProperty");
    }
#endif
    _bytes.reset(new IntrusiveBytes(_value.to_msgpack()));
  }

  return _bytes;
}

const Variant &ModelProperty::get_value() const {
  if (!_value_ready) {
#ifdef DSA_DEBUG
    if (_bytes == nullptr) {
      LOG_FATAL(Logger::_(), LOG << "invalid value in ModelProperty");
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

ref_<NodeModel> NodeModel::add_list_child(const std::string &name,
                                          ref_<NodeModel> &&model) {
  auto p = model.get();
  add_child(name, ModelRef(p));
  _list_children[name] = model;
  return std::move(model);
}

}  // namespace dsa
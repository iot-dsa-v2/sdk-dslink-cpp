#include "dsa_common.h"

#include "node_model.h"

#include "module/logger.h"
#include "node_state.h"
#include "stream/responder/outgoing_list_stream.h"

namespace dsa {

ModelProperty::ModelProperty(BytesRef &bytes)
    : _bytes(bytes), _value_ready(false) {}
ModelProperty::ModelProperty(Variant &&value)
    : _value(std::move(value)), _value_ready(true) {}

BytesRef &ModelProperty::get_bytes() const {
  if (_bytes == nullptr) {
    _bytes.reset(new IntrusiveBytes(_value.to_msgpack()));
  }
  return _bytes;
}

const Variant &ModelProperty::get_value() const {
  if (!_value_ready) {
    _value = Variant::from_msgpack(_bytes->data(), _bytes->size());
    _value_ready = true;
  }
  return _value;
}

void NodeModel::init_list_stream(OutgoingListStream &stream) {
  send_props_list(stream);
  send_children_list(stream);
}

void NodeModel::send_props_list(OutgoingListStream &stream) {
  for (auto & it : _metas) {
    stream.update_value(it.first, it.second.get_bytes());
  }
}
void NodeModel::send_children_list(OutgoingListStream &stream) {

}

}  // namespace dsa
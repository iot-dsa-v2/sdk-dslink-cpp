#include "dsa_common.h"

#include "value_node_model.h"

namespace dsa {
ValueNodeModel::ValueNodeModel(LinkStrandRef &&strand, Callback &&callback)
    : NodeModel(std::move(strand)), _callback(std::move(callback)) {}

void ValueNodeModel::destroy_impl() {
  _callback = nullptr;
  NodeModel::destroy_impl();
}

MessageStatus ValueNodeModel::on_set_value(MessageValue &&value) {
  if (_callback != nullptr && _callback(value.value)) {
    return NodeModel::on_set_value(std::move(value));
  }
  return MessageStatus::INVALID_PARAMETER;
}
}

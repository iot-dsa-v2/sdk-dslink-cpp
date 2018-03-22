#include "dsa_common.h"

#include "value_node_model.h"

namespace dsa {
ValueNodeModel::ValueNodeModel(const LinkStrandRef &strand, const string_ &type,
                               Callback &&callback,
                               PermissionLevel write_require_permission)
    : NodeModel(strand, write_require_permission),
      _callback(std::move(callback)) {
  update_property("$type", Var(type));
}

void ValueNodeModel::destroy_impl() {
  _callback = nullptr;
  NodeModel::destroy_impl();
}

StatusDetail ValueNodeModel::on_set_value(MessageValue &&value) {
  if (_callback != nullptr) {
    StatusDetail status = _callback(value.value);
    return NodeModel::on_set_value(std::move(value));
  }
  return MessageStatus::INVALID_PARAMETER;
}
}  // namespace dsa

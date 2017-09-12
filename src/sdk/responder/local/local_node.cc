#include "dsa_common.h"

#include "local_node.h"

namespace dsa {

void LocalNode::set_value(MessageValue &&value) {
  _value = std::move(value);
  _value_ready = true;
  if (_subscribe_callback != nullptr) {
    auto msg = make_ref_<SubscribeResponseMessage>();
    msg->set_value(MessageValue(_value));
    _subscribe_callback(std::move(msg));
  }
}

void LocalNode::subscribe(const SubscribeOptions &options,
                          SubscribeCallback &&callback) {
  NodeModel::subscribe(options, std::move(callback));
  if (_subscribe_callback != nullptr && _value_ready) {
    auto msg = make_ref_<SubscribeResponseMessage>();
    msg->set_value(MessageValue(_value));
    _subscribe_callback(std::move(msg));
  }
}
}
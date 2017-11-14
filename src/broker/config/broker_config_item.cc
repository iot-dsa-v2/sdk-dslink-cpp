#include "dsa_common.h"

#include "broker_config_item.h"

namespace dsa {
bool BrokerConfigItem::set_value(Var&& value) {
  if (_constraint >= Var::NUL && value.get_type() != _constraint) {
    return false;
  }
  // if (value == _value) return false;
  _value = value;
  for (auto& callback : _callbacks) {
    callback(_value);
  }
  return true;
}
std::list<BrokerConfigItem::Callback>::iterator BrokerConfigItem::listen(
    Callback&& callback) {
  _callbacks.push_back(std::move(callback));
  return _callbacks.end()--;
}
void BrokerConfigItem::cancel(std::list<Callback>::iterator& it) {
  _callbacks.erase(it);
}
}

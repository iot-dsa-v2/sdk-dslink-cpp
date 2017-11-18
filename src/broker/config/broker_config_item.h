#ifndef DSA_BROKER_CONFIG_ITEM_H
#define DSA_BROKER_CONFIG_ITEM_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>
#include <list>

#include "util/enable_ref.h"
#include "variant/variant.h"

namespace dsa {
class BrokerConfigItem {
 public:
  typedef std::function<void(const Var&)> Callback;

 private:
  Var _value;

  std::list<Callback> _callbacks;

  VarValidator _validator;

 public:
  Var& get_value() { return _value; }
  const Var& get_value() const { return _value; }

  BrokerConfigItem() {}
  BrokerConfigItem(Var&& value, VarValidator&& validator)
      : _value(std::move(value)), _validator(std::move(validator)) {}

  bool set_value(Var&& value);
  std::list<Callback>::iterator listen(Callback&& callback);
  void cancel(std::list<Callback>::iterator& it);
};
}

#endif  // DSA_BROKER_CONFIG_ITEM_H

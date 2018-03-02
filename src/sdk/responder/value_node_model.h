#ifndef DSA_SDK_VALUE_NODE_H
#define DSA_SDK_VALUE_NODE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>

#include "node_model.h"

namespace dsa {

class ValueNodeModel : public NodeModel {
 public:
  typedef std::function<bool(const Var &)> Callback;

 protected:
  Callback _callback;
  void destroy_impl() override;

 public:
  ValueNodeModel(
      LinkStrandRef &&strand, Callback &&callback,
      PermissionLevel write_require_permission = PermissionLevel::WRITE);

  MessageStatus on_set_value(MessageValue &&value) override;
};
}

#endif  // DSA_SDK_VALUE_NODE_H

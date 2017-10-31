#ifndef DSA_SDK_INVOKE_NODE_MODEL_H_
#define DSA_SDK_INVOKE_NODE_MODEL_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "node_model.h"

namespace dsa {

class InvokeNodeModel : public NodeModel {
  PermissionLevel _require_permission;

 public:
  InvokeNodeModel(LinkStrandRef &&strand, PermissionLevel require_permission = PermissionLevel::WRITE);

  void set_require_permission(PermissionLevel permission_level);

  // if you need to override invoke, extends from NodeModel instead of InvokeNodeModel
  void invoke(ref_<OutgoingInvokeStream> &&stream,
              ref_<NodeState> &parent) final;
  virtual void on_invoke(ref_<OutgoingInvokeStream> &&stream,
    ref_<NodeState> &parent){};
};

}  // namespace dsa

#endif  // DSA_SDK_INVOKE_NODE_MODEL_H_

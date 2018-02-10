#ifndef DSA_SDK_INVOKE_NODE_MODEL_H
#define DSA_SDK_INVOKE_NODE_MODEL_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>

#include "node_model.h"

namespace dsa {

class InvokeNodeModel : public NodeModel {
 private:
  PermissionLevel _invoke_require_permission = PermissionLevel::NEVER;
  void invoke_require_permission(PermissionLevel permission_level);

 public:
  explicit InvokeNodeModel(
      LinkStrandRef &&strand,
      PermissionLevel require_permission = PermissionLevel::WRITE);

 protected:
  // if you need to override invoke, extends from NodeModel instead of
  // InvokeNodeModel
  void invoke(ref_<OutgoingInvokeStream> &&stream,
              ref_<NodeState> &parent) final;

  virtual void on_invoke(ref_<OutgoingInvokeStream> &&stream,
                         ref_<NodeState> &parent){};
};

class SimpleInvokeNode : public InvokeNodeModel {
 public:
  // return int Var for a error status
  // return other type for normal invoke response
  typedef std::function<Var(Var &&)> SimpleCallback;

  typedef std::function<void(Var &&, SimpleInvokeNode &, OutgoingInvokeStream &,
                             ref_<NodeState> &&)>
      FullCallback;

  SimpleInvokeNode(LinkStrandRef &&strand, SimpleCallback &&callback,
                   PermissionLevel require_permission = PermissionLevel::WRITE);
  SimpleInvokeNode(LinkStrandRef &&strand, FullCallback &&callback,
                   PermissionLevel require_permission = PermissionLevel::WRITE);

 protected:
  SimpleCallback _simple_callback;
  FullCallback _full_callback;

  void on_invoke(ref_<OutgoingInvokeStream> &&stream,
                 ref_<NodeState> &parent) final;
};

}  // namespace dsa

#endif  // DSA_SDK_INVOKE_NODE_MODEL_H

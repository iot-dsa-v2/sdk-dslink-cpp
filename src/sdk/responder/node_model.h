#ifndef DSA_SDK_NODE_MODEL_H_
#define DSA_SDK_NODE_MODEL_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "model_base.h"

#include <unordered_map>

namespace dsa {

class NodeModel : public NodeModelBase {
 private:
  PermissionLevel _set_value_require_permission = PermissionLevel::NEVER;
  void set_value_require_permission(PermissionLevel permission_level);

 protected:
  std::unordered_map<string_, VarBytesRef> _metas;
  std::unordered_map<string_, VarBytesRef> _attributes;
  std::unordered_map<string_, ref_<NodeModelBase>> _list_children;

  VarBytesRef _summary;

  void destroy_impl() override;

  // initialize will be called when NodeModel is attached to NodeState
  void initialize() override;

 public:
  NodeModel(LinkStrandRef &&strand,
            PermissionLevel write_require_permission = PermissionLevel::NEVER);

  void on_list(BaseOutgoingListStream &stream, bool first_request) override;

  void update_property(const string_ &field, VarBytesRef &&value);
  void update_property(const string_ &field, Var &&value) {
    update_property(field, make_ref_<VarBytes>(std::move(value)));
  }
  ref_<NodeModelBase> add_list_child(const string_ &name,
                                     ref_<NodeModelBase> &&model);
  ref_<NodeModelBase> remove_list_child(const string_ name,
                         ref_<NodeModelBase> &&model);

  VarBytesRef &get_summary() override;

 protected:
  void on_subscribe(const SubscribeOptions &options,
                    bool first_request) override;

  void send_props_list(BaseOutgoingListStream &stream);
  void send_children_list(BaseOutgoingListStream &stream);

  void set(ref_<OutgoingSetStream> &&stream) override;
  virtual MessageStatus on_set_value(MessageValue &&value);
  virtual MessageStatus on_set_attribute(const string_ &field, Var &&value);
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_MANAGER_H_

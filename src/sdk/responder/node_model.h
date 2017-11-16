#ifndef DSA_SDK_NODE_MODEL_H_
#define DSA_SDK_NODE_MODEL_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "model_base.h"

#include <unordered_map>

namespace dsa {

class ModelProperty {
  mutable BytesRef _bytes;
  mutable Var _value;
  mutable bool _value_ready;

 public:
  ModelProperty();
  explicit ModelProperty(BytesRef &bytes);
  ModelProperty(Var &&value);

  BytesRef &get_bytes() const;
  const Var &get_value() const;
  bool valid() const { return _bytes != nullptr || _value_ready; }
};

class NodeModel : public NodeModelBase {
 private:
  PermissionLevel _set_value_require_permission = PermissionLevel::NEVER;
  void set_value_require_permission(PermissionLevel permission_level);

 protected:
  std::unordered_map<string_, ModelProperty> _metas;
  std::unordered_map<string_, ModelProperty> _attributes;
  std::unordered_map<string_, ref_<NodeModelBase>> _list_children;

  BytesRef _summary;

  void destroy_impl() override;

  // initialize will be called when NodeModel is attached to NodeState
  void initialize() override;

 public:
  NodeModel(LinkStrandRef &&strand,
            PermissionLevel write_require_permission = PermissionLevel::NEVER);

  void on_list(OutgoingListStream &stream, bool first_request) override;

  void update_property(const string_ &field, ModelProperty &&value);

  ref_<NodeModel> add_list_child(const string_ &name, ref_<NodeModel> &&model);

  BytesRef &get_summary() override;

 protected:
  void send_props_list(OutgoingListStream &stream);
  void send_children_list(OutgoingListStream &stream);

  void set(ref_<OutgoingSetStream> &&stream) override;
  virtual MessageStatus on_set_value(MessageValue &&value);
  virtual MessageStatus on_set_attribute(const string_ &field, Var &&value);
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_MANAGER_H_

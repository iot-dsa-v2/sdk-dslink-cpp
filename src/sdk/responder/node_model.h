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
 protected:
  std::unordered_map<std::string, ModelProperty> _metas;
  std::unordered_map<std::string, ModelProperty> _attributes;
  std::unordered_map<std::string, ref_<NodeModel>> _list_children;

  BytesRef _summary;

  void destroy_impl() override;
  void initialize() override;

 public:
  explicit NodeModel(LinkStrandRef &&strand)
      : NodeModelBase(std::move(strand)){};

  void on_list(OutgoingListStream &stream, bool first_request) override;

  void update_property(const std::string &field, ModelProperty &&value);

  ref_<NodeModel> add_list_child(const std::string &name,
                                 ref_<NodeModel> &&model);

  BytesRef &get_summary();

 protected:
  void send_props_list(OutgoingListStream &stream);
  void send_children_list(OutgoingListStream &stream);

  virtual bool allows_set_value() { return false; }

  void on_set(ref_<OutgoingSetStream> &&stream) override;
  virtual MessageStatus on_set_value(MessageValue &&value);
  virtual MessageStatus on_set_attribute(const std::string &field, Var &&value);
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_MANAGER_H_

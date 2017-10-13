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
  mutable Variant _value;
  mutable bool _value_ready;

 public:
  explicit ModelProperty(BytesRef &bytes);
  explicit ModelProperty(Variant &&value);

  BytesRef &get_bytes() const;
  const Variant &get_value() const;
};

class NodeModel : public NodeModelBase {
 public:
  explicit NodeModel(LinkStrandRef &&strand)
      : NodeModelBase(std::move(strand)){};

  std::unordered_map<std::string, ModelProperty> _metas;
  std::unordered_map<std::string, ModelProperty> _attributes;

  void init_list_stream(OutgoingListStream &stream) override;

 protected:
  void send_props_list(OutgoingListStream &stream);
  void send_children_list(OutgoingListStream &stream);
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_MANAGER_H_

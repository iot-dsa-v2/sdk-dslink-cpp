#ifndef DSA_SDK_LOCAL_NODE_H
#define DSA_SDK_LOCAL_NODE_H

#include <unordered_map>

#include "../node_model.h"

namespace dsa {

class LocalNode : public NodeModel {
 protected:
  MessageValue _value;
  bool _value_ready = false;

  std::unordered_map<std::string, ref_<LocalNode>> _children;

 public:
  const Path path;

  void set_value(MessageValue &&value);

  void on_subscribe(const SubscribeOptions &options) override;
};
}

#endif  // DSA_SDK_LOCAL_NODE_H

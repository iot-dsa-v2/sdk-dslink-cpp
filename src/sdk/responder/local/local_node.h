#ifndef DSA_SDK_LOCAL_NODE_H
#define DSA_SDK_LOCAL_NODE_H

#include "../node_model.h"

namespace dsa {

class LocalNode : public NodeModel {
 protected:
  MessageValue _value;
  bool _value_ready = false;

 public:
  const Path path;

  void set_value(MessageValue &&value);

  void subscribe(const SubscribeOptions &options,
                 SubscribeCallback &&callback) override;
};
}

#endif  // DSA_SDK_LOCAL_NODE_H

#ifndef DSA_SDK_NODE_MODEL_H_
#define DSA_SDK_NODE_MODEL_H_

#include <map>
#include <queue>

#include "message/base_message.h"
#include "util/enable_intrusive.h"
#include "node_state.h"

namespace dsa {
class NodeState;

// interface of the real model logic
class NodeModel : public EnableRef<NodeModel>  {
 private:

  ref_<NodeState> _state;
  std::map< size_t, ref_<MessageStream> > _invoke_streams;
  std::map< size_t, ref_<MessageStream> > _set_streams;


 public:

  void update_value(MessageValue new_value);
  void update_value(Variant new_value);
};

}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_H_
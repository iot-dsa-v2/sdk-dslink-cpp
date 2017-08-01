#ifndef DSA_SDK_NODE_MODEL_H_
#define DSA_SDK_NODE_MODEL_H_

#include <queue>
#include <map>
#include <mutex>

#include "dsa/util.h"
#include "outgoing_message_stream.h"

namespace dsa {
class NodeState;

// interface of the real model logic
class NodeModel {
 private:
  shared_ptr_<NodeState> _state;
  std::map<uint32_t, shared_ptr_<InvokeMessageStream>> _invoke_streams;
  std::map<uint32_t, shared_ptr_<SetMessageStream>> _set_streams;

  std::mutex _invoke_key;
  std::mutex _set_key;

  std::queue<SubscribeResponseMessage> _to_send;

 public:
  template <typename T>
  void update_value(T new_value);

  void add_invoke_stream(const shared_ptr_<InvokeMessageStream> &stream);
  void remove_invoke_stream(uint32_t request_id);

  void add_set_stream(const shared_ptr_<SetMessageStream> &stream);
  void remove_set_stream(uint32_t request_id);

};
}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_H_
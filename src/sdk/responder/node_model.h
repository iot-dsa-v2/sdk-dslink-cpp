#ifndef DSA_SDK_NODE_MODEL_H_
#define DSA_SDK_NODE_MODEL_H_

#include <map>
#include <mutex>
#include <queue>

#include "core/message_stream.h"
#include "util/enable_intrusive.h"
#include "outgoing_message_stream.h"

namespace dsa {
class NodeState;

// interface of the real model logic
class NodeModel : public StreamHolder {
 private:
  typedef intrusive_ptr_<MessageStream> stream_ptr_;

  intrusive_ptr_<NodeState> _state;
  std::map< size_t, intrusive_ptr_<MessageStream> > _invoke_streams;
  std::map< size_t, intrusive_ptr_<MessageStream> > _set_streams;

  std::mutex _invoke_key;
  std::mutex _set_key;

  std::queue<SubscribeResponseMessage> _to_send;

 public:
  template<typename T>
  void update_value(T new_value);

  void add_stream(const stream_ptr_ &stream) override;
  void remove_stream(const MessageStream *stream) override;
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_H_
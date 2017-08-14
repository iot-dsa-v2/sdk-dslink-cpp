#ifndef DSA_SDK_NODE_MODEL_H_
#define DSA_SDK_NODE_MODEL_H_

#include <map>
#include <mutex>
#include <queue>

#include "outgoing_message_stream.h"
#include "util/enable_intrusive.h"

namespace dsa {
class NodeState;

// interface of the real model logic
class NodeModel : public EnableIntrusive<NodeModel> {
 private:
  intrusive_ptr_<NodeState> _state;
  std::map< uint32_t, intrusive_ptr_<MessageStream> > _invoke_streams;
  std::map< uint32_t, intrusive_ptr_<MessageStream> > _set_streams;

  std::mutex _invoke_key;
  std::mutex _set_key;

  std::queue<SubscribeResponseMessage> _to_send;

 public:
  template<typename T>
  void update_value(T new_value);

  void new_invoke_stream(const intrusive_ptr_<Session> &session,
                         InvokeOptions &&config,
                         size_t unique_id,
                         uint32_t request_id);
  void remove_invoke_stream(uint32_t request_id);

  void new_set_stream(const intrusive_ptr_<Session> &session,
                      SetOptions &&config,
                      size_t unique_id,
                      uint32_t request_id);
  void remove_set_stream(uint32_t request_id);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_H_
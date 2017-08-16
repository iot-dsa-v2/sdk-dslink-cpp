#ifndef DSA_SDK_NODE_MODEL_H_
#define DSA_SDK_NODE_MODEL_H_

#include <map>
#include <queue>

#include "outgoing_message_stream.h"
#include "util/enable_intrusive.h"

namespace dsa {

// interface of the real model logic
class NodeModelBase : public EnableIntrusive<NodeModelBase> {
 protected:
  std::map<uint32_t, intrusive_ptr_<MessageStream> > _invoke_streams;
  std::map<uint32_t, intrusive_ptr_<MessageStream> > _set_streams;

  std::queue<SubscribeResponseMessage> _to_send;

public:
  void new_invoke_stream(const intrusive_ptr_<Session> &session,
                         InvokeOptions &&config, size_t unique_id,
                         uint32_t request_id);
  void remove_invoke_stream(uint32_t request_id);

  void new_set_stream(const intrusive_ptr_<Session> &session,
                      SetOptions &&config, size_t unique_id,
                      uint32_t request_id);
  void remove_set_stream(uint32_t request_id);

  virtual ~NodeModelBase() = default;
};

}  // namespace dsa

#include "node_state.h"

namespace dsa {
class NodeModel : public NodeModelBase {
  intrusive_ptr_<NodeStateBase> _state;
public:
  template<typename T>
  void update_value(T new_value);
};
}  // namespace dsa

#endif  // DSA_SDK_NODE_MODEL_H_
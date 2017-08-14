#include "dsa_common.h"

#include "node_model.h"

#include "node_state.h"
#include "core/session.h"

namespace dsa {

template <typename T>
void NodeModel::update_value(T new_value) {
  SubscribeResponseMessage message;
  message.set_status(MessageStatus::Ok);
  message.set_value(new_value, nullptr); // TODO: meta field needs to be changed once msgpack encoding is done
  if (_state != nullptr)
    _state->new_message(message);
}

void NodeModel::new_invoke_stream(const intrusive_ptr_<Session> &session,
                                  InvokeOptions &&config,
                                  size_t unique_id,
                                  uint32_t request_id) {
  _invoke_streams[request_id] = make_intrusive_<InvokeMessageStream>(session, &_invoke_streams, config, unique_id, request_id);
}

void NodeModel::remove_invoke_stream(uint32_t request_id) {
  std::lock_guard<std::mutex> lock(_invoke_key);
  _invoke_streams.erase(request_id);
}

void NodeModel::new_set_stream(const intrusive_ptr_<Session> &session,
                               SetOptions &&config,
                               size_t unique_id,
                               uint32_t request_id) {
  _set_streams[request_id] = make_intrusive_<SetMessageStream>(session, &_set_streams, config, unique_id, request_id);
}

void NodeModel::remove_set_stream(uint32_t request_id) {
  std::lock_guard<std::mutex> lock(_set_key);
  _set_streams.erase(request_id);
}

}  // namespace dsa
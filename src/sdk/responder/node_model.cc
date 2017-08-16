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

void NodeModelBase::new_invoke_stream(const intrusive_ptr_<Session> &session,
                                  InvokeOptions &&config,
                                  size_t unique_id,
                                  uint32_t request_id) {
  _invoke_streams[request_id] = make_intrusive_<InvokeMessageStream>(session, &_invoke_streams, config, unique_id, request_id);
}

void NodeModelBase::remove_invoke_stream(uint32_t request_id) {

  _invoke_streams.erase(request_id);
}

void NodeModelBase::new_set_stream(const intrusive_ptr_<Session> &session,
                               SetOptions &&config,
                               size_t unique_id,
                               uint32_t request_id) {
  _set_streams[request_id] = make_intrusive_<SetMessageStream>(session, &_set_streams, config, unique_id, request_id);
}

void NodeModelBase::remove_set_stream(uint32_t request_id) {

  _set_streams.erase(request_id);
}

}  // namespace dsa
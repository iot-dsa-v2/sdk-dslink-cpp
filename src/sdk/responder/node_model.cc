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

void NodeModel::add_stream(const intrusive_ptr_<MessageStream> &stream) {
  switch (stream->get_type()) {
    case StreamType::Invoke:_invoke_streams[stream->_unique_id] = stream;
      break;
    case StreamType::Set:_set_streams[stream->_unique_id] = stream;
      break;
    default:
      return;
  }

  stream->add_holder(intrusive_this());
}

void NodeModel::remove_stream(const MessageStream *stream) {
  if (stream == nullptr) return;

  switch (stream->get_type()) {
    case StreamType::Invoke:_invoke_streams.erase(stream->_unique_id);
      break;
    case StreamType::Set:_set_streams.erase(stream->_unique_id);
      break;
    default:return;
  }
}
}  // namespace dsa
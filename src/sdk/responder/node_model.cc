#include "dsa_common.h"

#include "node_model.h"

#include "core/session.h"
#include "util/date_time.h"

namespace dsa {


void NodeModel::update_value(MessageValue new_value) {
//  SubscribeResponseMessage message;
//  message.set_status(MessageStatus::OK);
//  message.set_value(std::move(new_value));
//  if (_state != nullptr)
//    _state->new_message(message);
}

void NodeModel::update_value(Variant new_value) {
//  SubscribeResponseMessage message;
//  message.set_status(MessageStatus::OK);
//  message.set_value(MessageValue(std::move(new_value), DateTime::get_ts()));
//  if (_state != nullptr)
//    _state->new_message(message);
}

//
//void NodeModel::add_stream(const ref_<MessageStream> &stream) {
//  switch (stream->get_type()) {
//    case StreamType::Invoke:_invoke_streams[stream->_unique_id] = stream;
//      break;
//    case StreamType::Set:_set_streams[stream->_unique_id] = stream;
//      break;
//    default:
//      return;
//  }
//
//  stream->add_holder(get_ref());
//}
//
//void NodeModel::remove_stream(const MessageStream *stream) {
//  if (stream == nullptr) return;
//
//  switch (stream->get_type()) {
//    case StreamType::Invoke:_invoke_streams.erase(stream->_unique_id);
//      break;
//    case StreamType::Set:_set_streams.erase(stream->_unique_id);
//      break;
//    default:return;
//  }
//}
}  // namespace dsa
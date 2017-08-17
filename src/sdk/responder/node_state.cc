#include "dsa_common.h"

#include "node_state.h"

#include "core/session.h"

namespace dsa {

NodeState::NodeState(boost::asio::io_service::strand &strand, std::string path)
    : _strand(strand), _path(std::move(path)) {}

void NodeState::new_message(const SubscribeResponseMessage &message) {
  _last_value.reset(new SubscribeResponseMessage(message));
  for (auto &it : _subscription_streams) {
    auto &stream = dynamic_cast<SubscribeMessageStream &>(*it.second);
    stream.new_message(message);
  }
}

void NodeState::add_stream(const stream_ptr_ &stream) {
  switch (stream->get_type()) {
    case StreamType::Subscribe:_subscription_streams[stream->_unique_id] = stream;
      break;
    case StreamType::List:_list_streams[stream->_unique_id] = stream;
      break;
    default:
      return;
  }
  stream->add_holder(intrusive_this<StreamHolder>());
}

void NodeState::remove_stream(const MessageStream *stream) {
  if (stream == nullptr) return;

  switch (stream->get_type()) {
    case StreamType::Subscribe:_subscription_streams.erase(stream->_unique_id);
      break;
    case StreamType::List:_list_streams.erase(stream->_unique_id);
      break;
    default:
      break;
  }
}

}  // namespace dsa
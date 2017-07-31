#include "dsa_common.h"

#include <boost/thread/locks.hpp>

#include "outgoing_message_stream.h"

namespace dsa {

SubscribeMessageStream::SubscribeMessageStream(const std::shared_ptr<Session> &session,
                                               SubscribeOptions config,
                                               size_t id,
                                               uint32_t rid)
    : OutgoingMessageStream(session, config.qos, id, rid), _config(config) {
  _set_ready = [=]() {
    session->add_ready_outgoing_stream(_request_id, _unique_id);
  };
}

void SubscribeMessageStream::new_message(const SubscribeResponseMessage &new_message) {
  {
    boost::unique_lock<boost::shared_mutex> lock(_key);
    _message_queue.push_front(new_message);
  }
  _set_ready();
}

size_t SubscribeMessageStream::get_next_message_size() {
  boost::shared_lock<boost::shared_mutex> lock(_key);
  return _message_queue.back().size();
}

const Message &SubscribeMessageStream::get_next_message() {
  boost::unique_lock<boost::shared_mutex> lock(_key);
  auto message = _message_queue.back();
  _message_queue.pop_back();
  return std::move(message);
}

}  // namespace dsa
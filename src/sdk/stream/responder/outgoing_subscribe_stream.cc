#include "dsa_common.h"

#include "outgoing_subscribe_stream.h"

#include <ctime>
#include "core/session.h"
#include "message/message_page_group.h"
#include "util/date_time.h"

namespace dsa {

static SubscribeOptions empty_subscribe_options(QosLevel::_0, -2);

static inline int32_t GET_REMAIN_SIZE(const ref_<const Message> &msg) {
  if (msg->type() == MessageType::PAGED) {
    return static_cast<const OutgoingPages *>(msg.get())->remain_size();
  }
  return msg->size();
}

OutgoingSubscribeStream::OutgoingSubscribeStream(ref_<Session> &&session,
                                                 const Path &path, uint32_t rid,
                                                 SubscribeOptions &&options)
    : MessageQueueStream(std::move(session), path, rid) {
  set_options(std::move(options));
}

void OutgoingSubscribeStream::destroy_impl() {
  if (_option_callback != nullptr) {
    _option_callback(*this, empty_subscribe_options);
    _option_callback = nullptr;
  };
}

void OutgoingSubscribeStream::check_queue_time(int64_t time) {
  if (_queue.size() <= 1) return;

  int64_t target_time = time - _max_queue_duration;
  for (auto it = _queue.begin(); it != _queue.end();) {
    if ((*it)->created_ts >= target_time) {
      _current_queue_time = (*it)->created_ts;
      return;
    }
    _current_queue_size -= GET_REMAIN_SIZE(*it);
    ++it;
    _queue.pop_front();
  }
}
void OutgoingSubscribeStream::check_queue_size() {
  if (_queue.size() <= 2) return;

  MessageCRef first_page_group;
  if (_queue.front()->type() == MessageType::PAGED) {
    auto *pages = DOWN_CAST<const OutgoingPages *>(_queue.front().get());
    if (pages->get_next_page() != nullptr &&
        pages->get_next_page()->get_page_id() > 0) {
      first_page_group = std::move(_queue.front());
      _queue.pop_front();
    }
  }
  std::deque<MessageCRef>::iterator it;
  std::deque<MessageCRef>::iterator end = _queue.end();
  end--;  // skip the last one from size check

  for (it = _queue.begin(); it != end;) {
    _current_queue_size -= GET_REMAIN_SIZE(*it);
    ++it;
    _queue.pop_front();
    if (_current_queue_size <= _max_queue_size) {
      break;
    }
  }
  if (first_page_group != nullptr) {
    _queue.push_front(std::move(first_page_group));
  } else if (it != _queue.end()) {
    // update current queue time
    _current_queue_time = (*it)->created_ts;
  }
}

void OutgoingSubscribeStream::set_options(SubscribeOptions &&options) {
  _options = std::move(options);
  _max_queue_size =
      _options.queue_size > 0 ? _options.queue_size : DEFAULT_MAX_QUEUE_SIZE;
  _max_queue_duration = _options.queue_duration > 0 ? _options.queue_duration
                                                    : DEFAULT_MAX_QUEUE_TIME;
  if (_max_queue_size < _current_queue_size) {
    check_queue_size();
  }
  auto current_time = std::time(nullptr);
  if (current_time - _max_queue_duration > _current_queue_time) {
    check_queue_time(current_time);
  }
}

void OutgoingSubscribeStream::receive_message(ref_<Message> &&message) {
  auto request_message =
      DOWN_CAST<const SubscribeRequestMessage *>(message.get());
  SubscribeOptions old_options = std::move(_options);
  set_options(request_message->get_subscribe_options());
  if (_option_callback != nullptr) {
    _option_callback(*this, old_options);
  }
}

void OutgoingSubscribeStream::update_response_status(Status status) {
  auto status_message = make_ref_<SubscribeResponseMessage>();
  status_message->set_status(status);
  send_message(std::move(status_message));
}
}

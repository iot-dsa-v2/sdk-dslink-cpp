#include "dsa_common.h"

#include "outgoing_subscribe_stream.h"

#include <ctime>
#include "util/date_time.h"

namespace dsa {

static SubscribeOptions empty_subscribe_options(QosLevel::_0, -2);

OutgoingSubscribeStream::OutgoingSubscribeStream(ref_<Session> &&session,
                                                 const Path &path, uint32_t rid,
                                                 SubscribeOptions &&options)
    : MessageQueueStream(std::move(session), path, rid) {
  set_options(std::move(options));
}

void OutgoingSubscribeStream::destroy_impl() {
  if (_option_callback != nullptr) {
    std::move(_option_callback)(*this, empty_subscribe_options);
  };
}

void OutgoingSubscribeStream::check_queue_time(int64_t time) {
  if (_queue.size() <= 1) return;

  int64_t target_time = time - _max_queue_duration;
  for (auto it = _queue.begin(); it != _queue.end(); ++it) {
    if ((*it)->created_ts >= target_time) {
      // clear all the data from the begin of the queue
      _queue.erase(_queue.begin(), it);
      _current_queue_time = (*it)->created_ts;
      return;
    }
    _current_queue_size -= (*it)->size();
  }
  // all message before target time, but still need to keep the last one
  purge();
}
void OutgoingSubscribeStream::check_queue_size() {
  for (auto it = _queue.begin(); it != _queue.end(); ++it) {
    _current_queue_size -= (*it)->size();
    if (_current_queue_size <= _max_queue_size) {
      // clear all the data from the begin of the queue
      _queue.erase(_queue.begin(), ++it);
      if (it != _queue.end()) {
        _current_queue_time = (*it)->created_ts;
      }
      return;
    }
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
}

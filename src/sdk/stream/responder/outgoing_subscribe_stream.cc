#include "dsa_common.h"

#include "outgoing_subscribe_stream.h"

#include "util/date_time.h"

namespace dsa {

static SubscribeOptions empty_subscribe_options;

OutgoingSubscribeStream::OutgoingSubscribeStream(ref_<Session> &&session,
                                                 const Path &path, uint32_t rid,
                                                 SubscribeOptions &&options)
    : MessageQueueStream(std::move(session), path, rid) {
  set_options(std::move(options));
}

void OutgoingSubscribeStream::close_impl() {
  if (_option_callback != nullptr) {
    std::move(_option_callback)(*this, empty_subscribe_options);
  };
}

void OutgoingSubscribeStream::check_queue_time() {}
void OutgoingSubscribeStream::check_queue_size() {
  int32_t size = 0;
  for (auto it = _queue.rbegin(); it != _queue.rend(); ++it) {
    int32_t nextsize = size + (*it)->size();
    if (nextsize > _max_queue_size) {
      // clear all the data from the begin of the queue
      _queue.erase(_queue.begin(), it.base());
      break;
    }
  }
  _current_queue_size = size;
}

void OutgoingSubscribeStream::set_options(SubscribeOptions &&options) {
  _options = std::move(options);
  _max_queue_size = _options.queue_size > 0 ? _options.queue_size : 65536;
  _max_queue_time = _options.queue_time > 0 ? _options.queue_time : 120;
  if (_max_queue_size < _current_queue_size) {
    check_queue_size();
  }
//  if (_max_queue_time < _current_queue_time) {
//    check_queue_time();
//  }
}

void OutgoingSubscribeStream::on_option_change(Callback &&callback) {
  _option_callback = callback;
}

void OutgoingSubscribeStream::receive_message(MessageCRef &&message) {
  auto request_message =
      DOWN_CAST<const SubscribeRequestMessage *>(message.get());
  SubscribeOptions old_options = std::move(_options);
  set_options(request_message->get_subscribe_options());
  if (_option_callback != nullptr) {
    _option_callback(*this, old_options);
  }
}

void OutgoingSubscribeStream::send_response(
    SubscribeResponseMessageCRef &&message) {
  _current_queue_size += message->size();
  send_message(std::move(message));
  if (_current_queue_size > _max_queue_size) {
    check_queue_size();
  }
}
}

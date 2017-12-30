#include "dsa_common.h"

#include "message_io_stream.h"

#include "core/session.h"
#include "message/message_page_group.h"

namespace dsa {

MessageRefedStream::MessageRefedStream(ref_<Session> &&session,
                                       const Path &path, uint32_t rid)
    : MessageStream(rid), path(path), _session(std::move(session)){};
MessageRefedStream::~MessageRefedStream() = default;

inline void MessageRefedStream::destroy_impl() { _session.reset(); }

void MessageRefedStream::send_message() {
  if (!_writing && !is_destroyed()) {
    _writing = true;
    _session->write_stream(get_ref());
  }
}
void MessageRefedStream::post_message() {
  if (!_writing && !is_destroyed()) {
    _writing = true;
    if (_session->is_writing()) {
      _session->write_stream(get_ref());
    } else {
      _session->get_strand()->post([ this, keepref = get_ref() ]() mutable {
        if (!is_destroyed()) {
          _session->write_stream(std::move(keepref));
        }
      });
    }
  }
}

void MessageRefedStream::make_critical() {
  _session->write_critical_stream(get_ref());
}
MessageCacheStream::MessageCacheStream(ref_<Session> &&session,
                                       const Path &path, uint32_t rid)
    : MessageRefedStream(std::move(session), path, rid) {}
MessageCacheStream::~MessageCacheStream() {}

void MessageCacheStream::destroy_impl() {
  _closed = true;
  _cache.reset();
  _session.reset();
  MessageRefedStream::destroy_impl();
}

void MessageCacheStream::send_message(MessageCRef &&msg, bool close) {
  if (msg == nullptr) return;
  if (_closed && !close) {
    return;
  }
  _cache = std::move(msg);
  if (!_writing && _cache != nullptr) {
    _writing = true;
    _session->write_stream(get_ref());
  }
}

size_t MessageCacheStream::peek_next_message_size(size_t available,
                                                  int64_t time) {
  if (is_destroyed() || _cache == nullptr) {
    _writing = false;
    return 0;
  }
  return _cache->size();
}
MessageCRef MessageCacheStream::get_next_message(AckCallback &callback) {
  _writing = false;
  if (is_destroyed() || _cache == nullptr) {
    return MessageCRef();
  }
  MessageCRef copy = std::move(_cache);
  check_close_message(copy);
  return std::move(copy);
}

MessageQueueStream::MessageQueueStream(ref_<Session> &&session,
                                       const Path &path, uint32_t rid)
    : MessageRefedStream(std::move(session), path, rid) {}
MessageQueueStream::~MessageQueueStream() {}

void MessageQueueStream::destroy_impl() {
  _closed = true;
  _queue.clear();
  _session.reset();
  MessageRefedStream::destroy_impl();
}

void MessageQueueStream::purge() {
  if (_queue.size() > 1) {
    MessageCRef last = std::move(_queue.back());
    _queue.clear();
    _current_queue_size = last->size();
    _current_queue_time = last->created_ts;
    _queue.emplace_back(std::move(last));
  }
}

void MessageQueueStream::send_message(MessageCRef &&msg, bool close) {
  if (msg == nullptr) return;
  if (_closed && !close) {
    return;
  }
  int64_t current_time = msg->created_ts;
  if (_queue.empty()) {
    _current_queue_time = current_time;
    _current_queue_size = msg->size();
  } else {
    _current_queue_size += msg->size();
  }

  _queue.emplace_back(std::move(msg));
  if (_current_queue_size > _max_queue_size) {
    check_queue_size();
  }
  if (_current_queue_time < current_time - _max_queue_duration) {
    check_queue_time(current_time);
  }

  if (!_writing) {
    _writing = true;
    _session->write_stream(get_ref());
  }
}

size_t MessageQueueStream::peek_next_message_size(size_t available,
                                                  int64_t time) {
  if (is_destroyed() || _queue.empty()) {
    _writing = false;
    return 0;
  }
  if (time - _max_queue_duration > _current_queue_time) {
    check_queue_time(time);
  }
  return _queue.front()->size();
}
MessageCRef MessageQueueStream::get_next_message(AckCallback &callback) {
  _writing = false;
  if (is_destroyed() || _queue.empty()) {
    return MessageCRef();
  }
  MessageCRef msg = std::move(_queue.front());
  _queue.pop_front();
  _current_queue_size -= msg->size();
  if (check_close_message(msg)) {
  } else if (!_queue.empty()) {
    _current_queue_time = _queue.front()->created_ts;
    _writing = true;
    _session->write_stream(get_ref());
  }
  return std::move(msg);
}
}

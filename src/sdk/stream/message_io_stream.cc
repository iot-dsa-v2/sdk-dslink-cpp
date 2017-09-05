#include "dsa_common.h"

#include "message_io_stream.h"

#include "core/session.h"

namespace dsa {

MessageRefedStream::MessageRefedStream(ref_<Session> &&session,
                                       const Path &path, uint32_t rid)
    : MessageStream(rid), path(path), _session(std::move(session)){};
MessageRefedStream::~MessageRefedStream() = default;

MessageCacheStream::MessageCacheStream(ref_<Session> &&session,
                                       const Path &path, uint32_t rid)
    : MessageRefedStream(std::move(session), path, rid) {}
MessageCacheStream::~MessageCacheStream() {}

void MessageCacheStream::close_impl() {
  _cache.reset();
  _session.reset();
}

void MessageCacheStream::send_message(MessageCRef &&msg) {
  _cache = std::move(msg);
  if (!_writing && _cache != nullptr && !is_closed()) {
    _writing = true;
    _session->write_stream(get_ref());
  }
}

size_t MessageCacheStream::peek_next_message_size(size_t available) {
  _writing = false;
  if (is_closed() || _cache == nullptr) {
    return 0;
  }
  return _cache->size();
}
MessageCRef MessageCacheStream::get_next_message(int32_t ack_id) {
  if (is_closed() || _cache == nullptr) {
    return nullptr;
  }
  return std::move(_cache);
}

MessageQueueStream::MessageQueueStream(ref_<Session> &&session,
                                       const Path &path, uint32_t rid)
    : MessageRefedStream(std::move(session), path, rid) {}
MessageQueueStream::~MessageQueueStream() {}

void MessageQueueStream::close_impl() {
  _queue.clear();
  _session.reset();
}
void MessageQueueStream::send_message(MessageCRef &&msg) {
  if (msg == nullptr || is_closed()) return;
  _queue.emplace_back(std::move(msg));
  if (!_writing) {
    _writing = true;
    _session->write_stream(get_ref());
  }
}

size_t MessageQueueStream::peek_next_message_size(size_t available) {
  _writing = false;
  if (is_closed() || _queue.empty()) {
    return 0;
  }
  return _queue.front()->size();
}
MessageCRef MessageQueueStream::get_next_message(int32_t ack_id) {
  if (is_closed() || _queue.empty()) {
    return nullptr;
  }
  MessageCRef msg = std::move(_queue.front());
  _queue.pop_front();

  if (!_queue.empty()) {
    _writing = true;
    _session->write_stream(get_ref());
  }
  return msg;
}
}

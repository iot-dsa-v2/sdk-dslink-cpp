#include "dsa_common.h"

#include "message_io_stream.h"

#include "core/session.h"

namespace dsa {

MessageCacheStream::MessageCacheStream(ref_<Session> &&session,
                                       const std::string &path, uint32_t rid)
    : MessageRefedStream(std::move(session), path, rid) {}
MessageCacheStream::~MessageCacheStream() {}

void MessageCacheStream::close_impl() {
  _cache.reset();
  _session.reset();
}

void MessageCacheStream::send_message(MessageRef &&msg) {
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
MessageRef MessageCacheStream::get_next_message() {
  if (is_closed() || _cache == nullptr) {
    return nullptr;
  }
  // same message can be shared between multiple stream,
  // need to update rid before writing
  _cache->set_rid(rid);
  return std::move(_cache);
}

MessageRefedStream::MessageRefedStream(ref_<Session> &&session,
                                       const std::string &path, uint32_t rid)
    : _session(std::move(session)), rid(rid){};

MessageQueueStream::MessageQueueStream(ref_<Session> &&session,
                                       const std::string &path, uint32_t rid)
    : MessageRefedStream(std::move(session), path, rid) {}
MessageQueueStream::~MessageQueueStream() {}

void MessageQueueStream::close_impl() {
  _queue.clear();
  _session.reset();
}
void MessageQueueStream::send_message(MessageRef &&msg) {
  if (msg == nullptr || is_closed()) return;
  _queue.push_back(std::move(msg));
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
MessageRef MessageQueueStream::get_next_message() {
  if (is_closed() || _queue.empty()) {
    return nullptr;
  }
  MessageRef msg = std::move(_queue.front());
  _queue.pop_front();

  if (!_queue.empty()) {
    _writing = true;
    _session->write_stream(get_ref());
  }
  // same message can be shared between multiple stream
  // need to update rid before writing
  msg->set_rid(rid);
  return msg;
}
}

#include "dsa_common.h"

#include "message_io_stream.h"

#include "core/session.h"
#include "message/message_page_group.h"
#include "module/logger.h"

namespace dsa {

static inline int32_t GET_REMAIN_SIZE(const ref_<const Message> &msg) {
  if (msg->type() == MessageType::PAGED) {
    return static_cast<const OutgoingPages *>(msg.get())->remain_size();
  }
  return msg->size();
}

MessageRefedStream::MessageRefedStream(ref_<BaseSession> &&session,
                                       const Path &path, uint32_t rid)
    : MessageStream(rid), path(path), _session(std::move(session)){};
MessageRefedStream::~MessageRefedStream() = default;

inline void MessageRefedStream::destroy_impl() { _session.reset(); }

void MessageRefedStream::send_message() {
  if (!_writing && !is_destroyed()) {
    _session->write_stream(get_ref());
  }
}

void MessageRefedStream::make_critical() {
  if (!is_destroyed()) {
    // if stream is destroyed, that means message is already sent
    // so there is no need to make it critical
    _session->write_critical_stream(get_ref());
  }
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
    _session->write_stream(get_ref());
  }
}

size_t MessageCacheStream::peek_next_message_size(size_t available,
                                                  int64_t time) {
  if (is_destroyed() || _cache == nullptr) {
    return 0;
  }
  return _cache->size();
}
MessageCRef MessageCacheStream::get_next_message(AckCallback &callback) {
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
  if (_queue.size() > 2) {
    if (_queue.size() == 3) {
      // remove the one in the middle
      MessageCRef last = std::move(_queue.back());
      _queue.pop_back();
      _queue.pop_back();
      _current_queue_size =
          GET_REMAIN_SIZE(*_queue.begin()) + GET_REMAIN_SIZE(last);
      _queue.emplace_back(std::move(last));
    } else {
      MessageCRef last = std::move(_queue.back());
      _queue.clear();
      _current_queue_size = GET_REMAIN_SIZE(last);
      _current_queue_time = last->created_ts;
      _queue.emplace_back(std::move(last));
    }
  }
}
void MessageQueueStream::check_queue_time(int64_t time) { purge(); }
void MessageQueueStream::check_queue_size() { purge(); }

void MessageQueueStream::send_message(MessageCRef &&msg, bool close) {
  if (msg == nullptr) return;
  if (_closed && !close) {
    return;
  }
  int64_t current_time = msg->created_ts;

  if (_waiting_page_group != nullptr) {
    if (_waiting_page_group->check_add(msg)) {
      _current_queue_size += msg->size();

      // TODO update current queue time?

      if (_waiting_page_group->is_ready()) {
        // no longer waiting
        _waiting_page_group.reset();
      }
      // skip the message adding logic
      // this message is already handled by queue
      goto CHECK_QUEUE;
    } else {
      _current_queue_size -= _waiting_page_group->remain_size();
      if (*(_queue.rbegin()) == _waiting_page_group) {
        _queue.pop_back();  // remove it from the queue
      } else {
        LOG_FATAL(
            __FILENAME__,
            LOG << "_waiting_page_group is not the last message in queue");
      }
      _waiting_page_group->drop();
      _waiting_page_group.reset();
    }
  }

  {
    // put these in scope to safely jump(goto) over them
    int32_t page_id = msg->get_page_id();
    if (page_id != 0) {
      if (page_id < 0) {
        // first page
        _waiting_page_group.reset(new OutgoingPages(std::move(msg)));
        msg = _waiting_page_group;

        if (_waiting_page_group->is_ready()) {
          // not waiting from the beginning
          // TODO, special optimization for the big message?
          _waiting_page_group.reset();
        }
      } else {
        // message group is dropped, ignore
        return;
      }
    }
  }
  if (_queue.empty()) {
    _current_queue_time = current_time;
    _current_queue_size = GET_REMAIN_SIZE(msg);
  } else {
    _current_queue_size += GET_REMAIN_SIZE(msg);
  }
  _queue.emplace_back(std::move(msg));

CHECK_QUEUE:

  if (_current_queue_size > _max_queue_size) {
    check_queue_size();
  }
  if (_current_queue_time < current_time - _max_queue_duration) {
    check_queue_time(current_time);
  }

  if (!_writing) {
    _session->write_stream(get_ref());
  }
}

size_t MessageQueueStream::peek_next_message_size(size_t available,
                                                  int64_t time) {
  if (is_destroyed() || _queue.empty()) {
    return 0;
  }
  if (time - _max_queue_duration > _current_queue_time) {
    check_queue_time(time);
  }
  if (_queue.front()->type() == MessageType::PAGED) {
    auto *pages = static_cast<const OutgoingPages *>(_queue.front().get());
    if (pages->is_ready() && pages->get_next_send() == nullptr) {
      // all pages are sent, remove from queue
      _queue.pop_front();
      // check the next message
      return peek_next_message_size(available, time);
    }
    return pages->next_size();
  }
  return _queue.front()->size();
}
MessageCRef MessageQueueStream::get_next_message(AckCallback &callback) {
  if (is_destroyed() || _queue.empty()) {
    return MessageCRef();
  }
  MessageCRef msg;
  if (_queue.front()->type() == MessageType::PAGED) {
    // OutgoingPages won't be shared between streams
    // so it's safe to do const_cast
    auto *cpages = static_cast<const OutgoingPages *>(_queue.front().get());
    auto *pages = const_cast<OutgoingPages *>(cpages);
    msg = pages->remove_next_send();
    // pages might be empty, handle this in the next peak_next_message_size call
  } else {
    msg = std::move(_queue.front());
    _queue.pop_front();
  }

  _current_queue_size -= msg->size();
  if (check_close_message(msg)) {
  } else if (!_queue.empty()) {
    // the queue might contain 1 or more empty OutgoingPages
    _current_queue_time = _queue.front()->created_ts;
    _session->write_stream(get_ref());
  }
  return std::move(msg);
}
}

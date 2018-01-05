#ifndef DSA_SDK_MESSAGE_PAGE_GROUP_H
#define DSA_SDK_MESSAGE_PAGE_GROUP_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <deque>
#include "base_message.h"

namespace dsa {

class OutgoingPages final : public Message {
  int32_t _rid;
  int32_t _sequence_id;
  int32_t _total_page;
  int32_t _waiting_page = 1;
  int32_t _remain_size;

  MessageCRef _next_send;
  MessageCRef _next_ack;

 public:
  OutgoingPages(MessageCRef &&msg);

  inline int32_t remain_size() const { return _remain_size; }
  int32_t next_size() const;

  bool check_add(MessageCRef &msg);
  const MessageCRef &get_next_send() const { return _next_send; }
  // remove a message from send queue
  MessageCRef remove_next_send();
  // all message received and ready to send
  bool is_ready() const { return _waiting_page >= _total_page; }
  // all messaged sent and acked
  bool is_done() const { return is_ready() && _next_ack == nullptr; }

  // destroy the message
  void drop();
};

// the main receiver stream should merge the paged message so their next values
// are connected as a linked list
class IncomingPagesMerger final : public EnableRef<IncomingPagesMerger> {
  int32_t _rid;
  int32_t _sequence_id;
  int32_t _total_page;
  int32_t _waiting_page = 1;

  MessageRef _current;

 public:
  IncomingPagesMerger(const ref_<Message> &msg);
  // return true when merger is no longer needed
  // either the merger is finished, or invalid
  bool check_merge(const ref_<Message> &msg);

  inline static void check_merge(ref_<IncomingPagesMerger> &pages,
                                 MessageRef &message) {
    if (message->get_page_id() < 0) {
      pages = make_ref_<IncomingPagesMerger>(message);
    } else if (pages != nullptr) {
      if (pages->check_merge(message)) {
        // merger is no longer needed
        pages = nullptr;
      }
    }
  }
};

// cache the paged value at handler side
// assume the pages are merged by a IncomingPagesMerger already
template <class MessageClass>
class IncomingPageCache final
    : public EnableRef<IncomingPageCache<MessageClass>> {
  int32_t _rid;
  int32_t _sequence_id;
  int32_t _last_page;

  ref_<const MessageClass> _first;

 public:
  explicit IncomingPageCache(ref_<const MessageClass> &&msg)
      : _rid(msg->get_rid()),
        _sequence_id(msg->get_sequence_id()),
        _last_page(-msg->get_page_id() - 1),
        _first(std::move(msg)){};

  // return the first message when ready
  // otherwise return nullptr
  inline static ref_<const MessageClass> get_first_page(
      ref_<IncomingPageCache<MessageClass>> &pages,
      ref_<const MessageClass> &&message) {
    int32_t page_id = message->get_page_id();
    if (pages != nullptr) {
      if (pages->_rid == message->get_rid() &&
          pages->_sequence_id == message->get_sequence_id()) {
        if (page_id >= pages->_last_page) {
          // page cache is done
          auto result = std::move(pages->_first);
          pages = nullptr;
          return std::move(result);
        }
        // page not ready
        return ref_<const MessageClass>();
      } else {
        // previous page is invalid
        pages.reset();
      }
    }
    if (page_id != 0) {
      if (page_id < 0) {
        pages = make_ref_<IncomingPageCache<MessageClass>>(std::move(message));
      }
    } else {
      return std::move(message);
    }
    return ref_<const MessageClass>();
  }
};
}
#endif  // DSA_SDK_MESSAGE_PAGE_GROUP_H

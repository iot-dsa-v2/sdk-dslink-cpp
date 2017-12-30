#ifndef DSA_SDK_MESSAGE_PAGE_GROUP_H
#define DSA_SDK_MESSAGE_PAGE_GROUP_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <deque>
#include "base_message.h"

namespace dsa {

class MesagePageGroup final : public Message {
  int32_t _sequence_id;
  int32_t _total_page;
  int32_t _waiting_page = 1;

  MessageCRef _next;

 public:
  MesagePageGroup(MessageCRef &&msg);

 public:
  bool push(MessageCRef &&msg);
  MessageCRef &get_next() { return _next; }
  MessageCRef remove_next();
  bool is_done() { return _waiting_page >= _total_page; }
};
}
#endif  // DSA_SDK_MESSAGE_PAGE_GROUP_H

#ifndef DSA_SDK_MESSAGE_PAGE_GROUP_H
#define DSA_SDK_MESSAGE_PAGE_GROUP_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <deque>
#include "base_message.h"

namespace dsa {

class OutgoingPages final : public Message {
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

class IncommingPages {};
}
#endif  // DSA_SDK_MESSAGE_PAGE_GROUP_H

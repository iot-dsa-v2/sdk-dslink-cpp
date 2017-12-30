#include "dsa_common.h"

#include "message_page_group.h"

namespace dsa {

MesagePageGroup::MesagePageGroup(MessageCRef&& msg)
    : Message(MessageType::PAGED),
      _sequence_id(msg->get_sequence_id()),
      _total_page(-msg->get_page_id()),
      _next(std::move(msg)) {}

bool MesagePageGroup::push(MessageCRef&& msg) {
  if (msg->get_sequence_id() != _sequence_id ||
      msg->get_page_id() != _waiting_page) {
    return false;
  }
  ++_waiting_page;
  return true;
}

MessageCRef MesagePageGroup::remove_next() {
  auto next = _next->get_next_page();
  auto result = std::move(_next);
  // next = next.next
  _next = std::move(next);
  return std::move(result);
}
}

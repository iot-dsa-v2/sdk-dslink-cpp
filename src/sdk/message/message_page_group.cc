#include "dsa_common.h"

#include "message_page_group.h"

namespace dsa {

OutgoingPages::OutgoingPages(MessageCRef&& msg)
    : Message(MessageType::PAGED),
      _rid(msg->get_rid()),
      _sequence_id(msg->get_sequence_id()),
      _total_page(-msg->get_page_id()),
      _next_send(std::move(msg)) {
  _remain_size = _next_send->size();
  if (_next_send->get_next_page() != nullptr) {
    const Message* p = _next_send->get_next_page().get();
    do {
      ++_waiting_page;
      _remain_size += p->size();
      p = p->get_next_page().get();
    } while (p != nullptr);
  }
}

int32_t OutgoingPages::next_size() const {
  if (_next_send != nullptr) {
    return _next_send->size();
  }
  return 0;
}

bool OutgoingPages::check_add(MessageCRef& msg) {
  if (msg->get_rid() != _rid || msg->get_sequence_id() != _sequence_id ||
      msg->get_page_id() != _waiting_page) {
    return false;
  }
  _remain_size += msg->size();
  ++_waiting_page;
  if (_next_send == nullptr) {
    _next_send = msg;
    created_ts = _next_send->created_ts;
  }
  return true;
}

MessageCRef OutgoingPages::remove_next_send() {
  ref_<const Message> next = _next_send->get_next_page();
  auto result = std::move(_next_send);
  // next = next.next
  _next_send = std::move(next);
  if (_next_send != nullptr) {
    created_ts = _next_send->created_ts;
  }
  if (_remain_size) {
    _remain_size -= result->size();
  }
  return std::move(result);
}

void OutgoingPages::drop() {
  _next_send.reset();
  _total_page = 0;
  _waiting_page = 0;
  _remain_size = 0;
}

IncomingPages::IncomingPages(ref_<Message>& msg)
    : _rid(msg->get_rid()),
      _sequence_id(msg->get_sequence_id()),
      _total_page(-msg->get_page_id()),
      first(msg),
      _current(msg) {}

bool IncomingPages::check_add(ref_<Message>& msg) {
  if (msg->get_rid() != _rid || msg->get_sequence_id() != _sequence_id ||
      msg->get_page_id() != _waiting_page) {
    return false;
  }
  ++_waiting_page;
  _current = msg;
  return true;
}
}

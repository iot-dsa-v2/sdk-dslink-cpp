#include "base_message.h"

namespace dsa {
Message::Message(const SharedBuffer& buffer) : static_headers(buffer.data){};

void Message::write(uint8_t* data) const throw(const std::runtime_error&) {
  if (!static_headers.message_size) {
    // message_size shouldn't be 0
    throw std::runtime_error("invalid message size");
  }
  static_headers.write(data);
  write_dynamic_data(data + StaticHeaders::TotalSize);
}

uint8_t Message::get_priority() const {
  if (priority == nullptr) return 0;
  return priority->value();
}
void Message::set_priority(uint8_t value) {
  if (value == 0x80) {
    // default priority, remove
    if (priority != nullptr) {
      priority.reset();
      // invalidate message size
      static_headers.message_size = 0;
    }

  } else {
    if (priority != nullptr) {
      // invalidate message size
      static_headers.message_size = 0;
    }
    priority.reset(new DynamicByteHeader(DynamicHeader::Priority, value));
  }
}

int32_t Message::get_sequence_id() const {
  if (sequence_id == nullptr) return 0;
  return sequence_id->value();
}
void Message::set_sequence_id(int32_t value) {
  if (value == 0) {
    // default priority, remove
    if (sequence_id != nullptr) {
      sequence_id.reset();
      // invalidate message size
      static_headers.message_size = 0;
    }

  } else {
    if (sequence_id != nullptr) {
      // invalidate message size
      static_headers.message_size = 0;
    }
    sequence_id.reset(new DynamicIntHeader(DynamicHeader::SequenceId, value));
  }
}

int32_t Message::get_page_id() const {
  if (page_id == nullptr) return 0;
  return page_id->value();
}
void Message::set_page_id(int32_t value) {
  if (value == 0) {
    // default priority, remove
    if (page_id != nullptr) {
      page_id.reset();
      // invalidate message size
      static_headers.message_size = 0;
    }

  } else {
    if (page_id != nullptr) {
      // invalidate message size
      static_headers.message_size = 0;
    }
    page_id.reset(new DynamicIntHeader(DynamicHeader::PageId, value));
  }
}

RequestMessage::RequestMessage(const SharedBuffer& buffer) : Message(buffer){};
ResponseMessage::ResponseMessage(const SharedBuffer& buffer)
    : Message(buffer){};
}  // namespace dsa

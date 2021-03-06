#include "dsa_common.h"

#include "invoke_request_message.h"

#include "variant/variant.h"

namespace dsa {

InvokeRequestMessage::InvokeRequestMessage(const uint8_t* data, size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

InvokeRequestMessage::InvokeRequestMessage()
    : RequestMessage(MessageType::INVOKE_REQUEST) {}

InvokeRequestMessage::InvokeRequestMessage(const string_& path, Var&& value)
    : RequestMessage(MessageType::INVOKE_REQUEST) {
  set_target_path(path);
  set_value(std::move(value));
}

bool InvokeRequestMessage::set_value(const Var& value, int32_t sequence_id) {
  auto msgpack = value.to_msgpack();
  if (msgpack.size() > Var::MAX_PAGE_BODY_SIZE) {
    std::vector<BytesRef> pages = Var::split_pages(msgpack);
    set_body(std::move(*pages.begin()));
    set_page_id(-pages.size());
    set_sequence_id(sequence_id);
    auto* current = this;
    for (int32_t i = 1; i < pages.size(); ++i) {
      auto next = make_ref_<InvokeRequestMessage>();
      next->set_body(std::move(pages[i]));
      next->set_page_id(i);
      next->set_sequence_id(sequence_id);
      auto* p_next = next.get();
      current->set_next_page(std::move(next));
      current = p_next;
    }
    return true;
  } else {
    set_body(std::move(msgpack));
    return false;
  }
}
Var InvokeRequestMessage::get_value() const {
  if (_next_page != nullptr && get_page_id() < 0) {
    std::vector<BytesRef> pages;
    const Message* current = this;
    while (current != nullptr) {
      pages.emplace_back(current->get_body());
      current = current->get_next_page().get();
    }
    return Var::from_msgpack_pages(pages);
  } else if (body != nullptr && !body->empty()) {
    return Var::from_msgpack(body->data(), body->size());
  }
  return Var();
}

const PermissionLevel InvokeRequestMessage::get_max_permission() const {
  return static_cast<PermissionLevel>(
      DynamicByteHeader::read_value(max_permission));
}
void InvokeRequestMessage::set_max_permission(PermissionLevel value) {
  if (DynamicByteHeader::write_value(max_permission,
                                     DynamicHeader::MAX_PERMISSION,
                                     static_cast<uint8_t>(value))) {
    static_headers.message_size = 0;
  }
}

const bool InvokeRequestMessage::get_skippable() const {
  return DynamicBoolHeader::read_value(skippable);
}
void InvokeRequestMessage::set_skippable(bool value) {
  if (DynamicBoolHeader::write_value(skippable, DynamicHeader::SKIPPABLE,
                                     value)) {
    static_headers.message_size = 0;
  }
}

const bool InvokeRequestMessage::get_refreshed() const {
  return DynamicBoolHeader::read_value(refreshed);
}
void InvokeRequestMessage::set_refreshed(bool value) {
  if (DynamicBoolHeader::write_value(refreshed, DynamicHeader::REFRESHED,
                                     value)) {
    static_headers.message_size = 0;
  }
}

}  // namespace dsa

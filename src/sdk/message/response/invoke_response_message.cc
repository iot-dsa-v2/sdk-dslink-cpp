#include "dsa_common.h"

#include "invoke_response_message.h"

#include "variant/variant.h"

namespace dsa {
InvokeResponseMessage::InvokeResponseMessage(const uint8_t* data, size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

InvokeResponseMessage::InvokeResponseMessage()
    : ResponseMessage(MessageType::INVOKE_RESPONSE) {}

bool InvokeResponseMessage::set_value(const Var& value, int32_t sequence_id) {
  auto msgpack = value.to_msgpack();
  if (msgpack.size() > Var::MAX_PAGE_BODY_SIZE) {
    std::vector<BytesRef> pages = Var::split_pages(msgpack);
    set_body(std::move(*pages.begin()));
    set_page_id(-pages.size());
    set_sequence_id(sequence_id);
    auto* current = this;
    for (int32_t i = 1; i < pages.size(); ++i) {
      auto next = make_ref_<InvokeResponseMessage>();
      next->set_body(std::move(pages[i]));
      next->set_page_id(i);
      next->set_sequence_id(sequence_id);
      auto* p_next = next.get();
      current->set_next_page(std::move(next));
      current = p_next;
    }
    // move the status to the last
    current->set_status(get_status());
    set_status(Status::OK);
    return true;
  } else {
    set_body(std::move(msgpack));
    return false;
  }
}
Var InvokeResponseMessage::get_value() const {
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

const bool InvokeResponseMessage::get_skippable() const {
  return DynamicBoolHeader::read_value(skippable);
}
void InvokeResponseMessage::set_skippable(bool value) {
  if (DynamicBoolHeader::write_value(skippable, DynamicHeader::SKIPPABLE,
                                     value)) {
    static_headers.message_size = 0;
  }
}

const bool InvokeResponseMessage::get_refreshed() const {
  return DynamicBoolHeader::read_value(refreshed);
}
void InvokeResponseMessage::set_refreshed(bool value) {
  if (DynamicBoolHeader::write_value(refreshed, DynamicHeader::REFRESHED,
                                     value)) {
    static_headers.message_size = 0;
  }
}

const string_& InvokeResponseMessage::get_audit_log() const {
  return DynamicStringHeader::read_value(audit_log);
}
void InvokeResponseMessage::set_audit_log(const string_& value) {
  if (DynamicStringHeader::write_value(audit_log, DynamicHeader::AUDIT_LOG,
                                       value)) {
    static_headers.message_size = 0;
  }
}

const string_& InvokeResponseMessage::get_error_detail() const {
  return DynamicStringHeader::read_value(error_detail);
}
void InvokeResponseMessage::set_error_detail(const string_& value) {
  if (DynamicStringHeader::write_value(error_detail,
                                       DynamicHeader::ERROR_DETAIL, value)) {
    static_headers.message_size = 0;
  }
}

}  // namespace dsa

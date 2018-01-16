#include "dsa_common.h"

#include "message_value.h"

#include "message/base_message.h"
#include "message/request/set_request_message.h"
#include "message/response/subscribe_response_message.h"
#include "util/little_endian.h"

namespace dsa {

MessageValue::MessageValue(const Var& value) : value(value) {}
MessageValue::MessageValue(Var&& value) : value(std::move(value)) {}

MessageValue::MessageValue(const Message* message) { parse(message); }

MessageValue::MessageValue(Var&& value, const string_& ts)
    : value(std::move(value)), meta({{"ts", Var(ts)}}) {}

void MessageValue::parse(const Message* message) {
  if (message->get_body() == nullptr) {
    return;
  }
  const uint8_t* data = message->get_body()->data();
  size_t size = message->get_body()->size();
  if (size < 2) {
    return;
  }
  uint16_t meta_size;
  memcpy(&meta_size, data, sizeof(uint16_t));
  data += sizeof(meta_size);
  size -= sizeof(meta_size);

  if (meta_size > size) {
    return;
  }

  meta = std::move(Var::from_msgpack(data, meta_size));
  data += meta_size;
  size -= meta_size;
  if (message->get_next_page() != nullptr) {
    // parse multi-page message
    std::vector<BytesRef> pages;
    const Message* current = message->get_next_page().get();
    while (current != nullptr) {
      pages.emplace_back(current->get_body());
      current = current->get_next_page().get();
    }
    value = Var::from_msgpack_pages(pages, data, size);
  } else if (size > 0) {
    value = std::move(Var::from_msgpack(data, size));
  }
}
template <class MessageClass>
MessageClass* MessageValue::write(MessageClass* message,
                                  int32_t sequence_id) const {
  std::vector<uint8_t> meta_bytes;
  std::vector<uint8_t> value_bytes;

  if (!value.is_null()) {
    value_bytes = value.to_msgpack();
  }

  if (meta.is_map()) {
    meta_bytes = meta.to_msgpack();
  }

  // return nullptr for single page
  MessageClass* last_page = nullptr;

  if (value_bytes.size() + meta_bytes.size() + 2 > Var::MAX_PAGE_BODY_SIZE) {
    if (meta_bytes.size() > Var::MAX_PAGE_BODY_SIZE) {
      throw std::runtime_error("value meta data is too big");
    }
    auto pages = Var::split_pages(
        value_bytes, Var::MAX_PAGE_BODY_SIZE - meta_bytes.size() - 2);
    // update value_bytes to reuse same logic to encode first page
    value_bytes = std::move(const_cast<RefCountBytes&>(*pages[0]));
    message->set_page_id(-pages.size());
    last_page = message;
    for (int32_t i = 1; i < pages.size(); ++i) {
      auto next = make_ref_<MessageClass>();
      next->set_body(std::move(pages[i]));
      next->set_page_id(i);
      next->set_sequence_id(sequence_id);
      auto* p_next = next.get();
      last_page->set_next_page(std::move(next));
      if (p_next) last_page = p_next;
    }
  }

  auto merged = new RefCountBytes();
  merged->resize(2);
  merged->reserve(2 + meta_bytes.size() + value_bytes.size());

  write_16_t(merged->data(), meta_bytes.size());
  merged->insert(merged->end(), meta_bytes.begin(), meta_bytes.end());
  merged->insert(merged->end(), value_bytes.begin(), value_bytes.end());
  message->set_body(std::move(merged));
  return last_page;
}

// implement template functions
void _implement_MessageValue_template_function() {
  MessageValue v;
  v.write<SubscribeResponseMessage>(nullptr);
  v.write<SetRequestMessage>(nullptr);
  throw "_implement_MessageValue_template_function is called at runtime";
}

}  // namespace dsa

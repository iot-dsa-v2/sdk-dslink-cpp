#include "dsa_common.h"

#include "subscribe_response_message.h"

#include "util/date_time.h"

namespace dsa {
SubscribeResponseMessage::SubscribeResponseMessage(const uint8_t* data,
                                                   size_t size)
    : ResponseMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
  if (decode_all && get_page_id() == 0) {
    _cached_value.reset(new MessageValue(this));
  }
}

SubscribeResponseMessage::SubscribeResponseMessage()
    : ResponseMessage(MessageType::SUBSCRIBE_RESPONSE) {}

SubscribeResponseMessage::SubscribeResponseMessage(Var&& value)
    : ResponseMessage(MessageType::SUBSCRIBE_RESPONSE) {
  set_value(MessageValue(std::move(value), DateTime::get_ts()));
}

MessageValue SubscribeResponseMessage::get_value() const {
  if (_cached_value != nullptr) {
    MessageValue result = std::move(*_cached_value);
    _cached_value.reset();
    return std::move(result);
  }
  return MessageValue(this);
}
void SubscribeResponseMessage::set_value(MessageValue&& value,
                                         int32_t sequence_id) {
  SubscribeResponseMessage* last_page = value.write(this, sequence_id);
  if (last_page != nullptr) {
    // move the status to the last page
    last_page->set_status(get_status());
    set_status(MessageStatus::OK);
  }

  // invalidate message_size
  static_headers.message_size = 0;
}

MergeQueueResult SubscribeResponseMessage::merge_queue(
    ref_<const Message>& next) {
  if (body == nullptr) {
    return MergeQueueResult::SKIP_THIS;
  } else if (next->get_body() == nullptr &&
             static_cast<const ResponseMessage*>(next.get())->get_status() ==
                 get_status()) {
    return MergeQueueResult::SKIP_NEXT;
  }
  return MergeQueueResult::NORMAL;
}

void SubscribeResponseMessage::print_body(std::ostream& os) const {
  if (body != nullptr && body->size() > 0) {
    if (body->size() < 256) {
      MessageValue value = get_value();
      if (value.meta.is_map()) {
        os << " META: " << value.meta.to_json();
      }
      os << " VALUE: " << value.value.to_json();
    } else {
      os << " BODY SIZE: " << body->size();
    }
  }
}

}  // namespace dsa

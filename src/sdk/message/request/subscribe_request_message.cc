#include "subscribe_request_message.h"

namespace dsa {
SubscribeRequestMessage::SubscribeRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

SubscribeOption::Qos SubscribeRequestMessage::get_qos() const {
  return static_cast<SubscribeOption::Qos>(DynamicByteHeader::read_value(qos));
}
void SubscribeRequestMessage::set_qos(SubscribeOption::Qos value) {
  if (DynamicByteHeader::write_value(qos, DynamicHeader::Qos, value)) {
    static_headers.message_size = 0;
  }
}

int32_t SubscribeRequestMessage::get_queue_size() const {
  return DynamicIntHeader::read_value(queue_size);
}
void SubscribeRequestMessage::set_queue_size(int32_t value) {
  if (DynamicIntHeader::write_value(queue_size, DynamicHeader::QueueSize,
                                    value)) {
    static_headers.message_size = 0;
  }
}

int32_t SubscribeRequestMessage::get_queue_time() const {
  return DynamicIntHeader::read_value(queue_time);
}
void SubscribeRequestMessage::set_queue_time(int32_t value) {
  if (DynamicIntHeader::write_value(queue_time, DynamicHeader::QueueTime,
                                    value)) {
    static_headers.message_size = 0;
  }
}

SubscribeOption SubscribeRequestMessage::get_subscribe_option() const {
  return SubscribeOption(get_qos(), get_queue_size(), get_queue_time());
}
void SubscribeRequestMessage::set_subscribe_option(
    const SubscribeOption& option) {}

/*

uint8_t ResponseMessage::get_status() const {
  return DynamicByteHeader::read_value(status);
}
void ResponseMessage::set_status(uint8_t value) {
  if (DynamicByteHeader::write_value(status, DynamicHeader::Status, value)) {
    static_headers.message_size = 0;
  }
}*/

}  // namespace dsa

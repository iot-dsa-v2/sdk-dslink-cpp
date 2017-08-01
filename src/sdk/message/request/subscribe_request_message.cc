#include "dsa_common.h"

#include "subscribe_request_message.h"

namespace dsa {

SubscribeRequestMessage::SubscribeRequestMessage(const SharedBuffer& buffer)
    : RequestMessage(buffer) {
  parse_dynamic_headers(buffer.data + StaticHeaders::TotalSize,
                        static_headers.header_size - StaticHeaders::TotalSize);
}

SubscribeRequestMessage::SubscribeRequestMessage() : RequestMessage(MessageType::SubscribeRequest){};

StreamQos SubscribeRequestMessage::get_qos() const {
  return static_cast<StreamQos>(DynamicByteHeader::read_value(qos));
}
void SubscribeRequestMessage::set_qos(StreamQos value) {
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

SubscribeOptions SubscribeRequestMessage::get_subscribe_options() const {
  return SubscribeOptions(get_qos(), get_queue_size(), get_queue_time());
}
void SubscribeRequestMessage::set_subscribe_option(
    const SubscribeOptions& option) {}

}  // namespace dsa

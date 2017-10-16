#include "dsa_common.h"

#include "subscribe_request_message.h"

namespace dsa {

SubscribeRequestMessage::SubscribeRequestMessage(const uint8_t* data,
                                                 size_t size)
    : RequestMessage(data, size) {
  parse_dynamic_data(data + StaticHeaders::TOTAL_SIZE,
                     static_headers.header_size - StaticHeaders::TOTAL_SIZE,
                     size - static_headers.header_size);
}

SubscribeRequestMessage::SubscribeRequestMessage()
    : RequestMessage(MessageType::SUBSCRIBE_REQUEST){};

QosLevel SubscribeRequestMessage::get_qos() const {
  return static_cast<QosLevel>(DynamicByteHeader::read_value(qos));
}
void SubscribeRequestMessage::set_qos(QosLevel value) {
  if (DynamicByteHeader::write_value(qos, DynamicHeader::QOS, value)) {
    static_headers.message_size = 0;
  }
}

int32_t SubscribeRequestMessage::get_queue_size() const {
  return DynamicIntHeader::read_value(queue_size);
}
void SubscribeRequestMessage::set_queue_size(int32_t value) {
  if (DynamicIntHeader::write_value(queue_size, DynamicHeader::QUEUE_SIZE,
                                    value)) {
    static_headers.message_size = 0;
  }
}

int32_t SubscribeRequestMessage::get_queue_time() const {
  return DynamicIntHeader::read_value(queue_time);
}
void SubscribeRequestMessage::set_queue_time(int32_t value) {
  if (DynamicIntHeader::write_value(queue_time, DynamicHeader::QUEUE_TIME,
                                    value)) {
    static_headers.message_size = 0;
  }
}

SubscribeOptions SubscribeRequestMessage::get_subscribe_options() const {
  return SubscribeOptions(get_qos(), get_queue_size(), get_queue_time(),
                          get_priority());
}
void SubscribeRequestMessage::set_subscribe_option(
    const SubscribeOptions& options) {
  set_priority(options.priority);
  set_qos(options.qos);
  set_queue_size(options.queue_size);
  set_queue_time(options.queue_duration);
}

}  // namespace dsa

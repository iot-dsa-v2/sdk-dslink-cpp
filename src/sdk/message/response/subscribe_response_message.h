#ifndef DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_
#define DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../../util/buffer.h"
#include "../base_message.h"
#include "message/message_value.h"

namespace dsa {

class SubscribeResponseMessage final : public ResponseMessage {
 public:
  explicit SubscribeResponseMessage(const uint8_t* data, size_t size);
  SubscribeResponseMessage(const SubscribeResponseMessage&);
  SubscribeResponseMessage();
  explicit SubscribeResponseMessage(Var&& value);

 public:

  MessageValue get_value() const;
  void set_value(MessageValue&& value, int32_t sequence_id = 0);

  MergeQueueResult merge_queue(ref_<const Message>& next) final;

 protected:
  mutable std::unique_ptr<MessageValue> _cached_value;

  // measure the size and header size
  void update_static_header() final;
  void print_headers(std::ostream &os) const final;
  void print_body(std::ostream& os) const final;

  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const final;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);
};

typedef ref_<const SubscribeResponseMessage> SubscribeResponseMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_

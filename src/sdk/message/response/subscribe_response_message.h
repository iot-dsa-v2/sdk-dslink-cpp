#ifndef DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_
#define DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"
#include "message/message_value.h"

namespace dsa {

class SubscribeResponseMessage : public ResponseMessage, PagedMessageMixin {
 private:
  mutable MessageValue _parsed_value;
  mutable bool _parsed = false;

 public:
  explicit SubscribeResponseMessage(const uint8_t* data, size_t size);
  SubscribeResponseMessage(const SubscribeResponseMessage&);
  SubscribeResponseMessage();
  SubscribeResponseMessage(Variant&& value);

 public:
  const MessageValue& get_value() const;
  void set_value(MessageValue&& value);

 protected:
  // measure the size and header size
  void update_static_header() override;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const override;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);
};

typedef ref_<const SubscribeResponseMessage> SubscribeResponseMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_

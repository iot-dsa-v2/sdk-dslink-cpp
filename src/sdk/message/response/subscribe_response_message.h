#ifndef DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_
#define DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"
#include "../parsed_message_value.h"

namespace dsa {

class SubscribeResponseMessage : public ResponseMessage, PagedMessageMixin {
 private:
  std::unique_ptr<MessageValue> _parsed_value;

 public:
  explicit SubscribeResponseMessage(const uint8_t* data, size_t size);
  SubscribeResponseMessage(const SubscribeResponseMessage&);
  SubscribeResponseMessage();

 public:
  const MessageValue& get_value();
  void set_value(const Variant & value, const Variant & meta);

 protected:
  // measure the size and header size
  void update_static_header() override;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const override;
  void parse_dynamic_headers(const uint8_t* data, size_t size) throw(const MessageParsingError &);
};

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_

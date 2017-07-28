#ifndef DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_
#define DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"
#include "../parsed_message_value.h"

namespace dsa {

class SubscribeResponseMessage : public ResponseMessage {
 private:
  std::unique_ptr<ParsedMessageValue> _parsed_value;

 public:
  SubscribeResponseMessage(const SharedBuffer& buffer);
  SubscribeResponseMessage();
  SubscribeResponseMessage(const SubscribeResponseMessage&);

 public:
  // measure the size and header size
  void update_static_header();

  const ParsedMessageValue* get_value();

 protected:
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_headers(const uint8_t* data, size_t size);
};

}  // namespace dsa

#endif  // DSA_SDK_SUBSCRIBE_RESPONSE_MESSAGE_H_

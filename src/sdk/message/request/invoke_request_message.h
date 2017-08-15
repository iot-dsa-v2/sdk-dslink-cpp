#ifndef DSA_SDK_INVOKE_REQUEST_MESSAGE_H_
#define DSA_SDK_INVOKE_REQUEST_MESSAGE_H_

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class InvokeRequestMessage : public RequestMessage, PagedMessageMixin {
 public:
  InvokeRequestMessage(const uint8_t* begin, const uint8_t* end);
  InvokeRequestMessage();
  InvokeRequestMessage(const InvokeRequestMessage&);

  std::unique_ptr<DynamicIntHeader> sequence_id;
  std::unique_ptr<DynamicByteHeader> max_permission;

 protected:
  // measure the size and header size
  void update_static_header();
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_headers(const uint8_t* data, size_t size) throw(const MessageParsingError &);
};

}  // namespace dsa

#endif  // DSA_SDK_INVOKE_REQUEST_MESSAGE_H_

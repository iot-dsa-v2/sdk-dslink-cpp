#ifndef DSA_SDK_LIST_REQUEST_MESSAGE_H_
#define DSA_SDK_LIST_REQUEST_MESSAGE_H_

#include <message/message_options.h>
#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class ListRequestMessage : public RequestMessage {
 public:
  ListRequestMessage(const uint8_t* data, size_t size);
  ListRequestMessage();
  ListRequestMessage(const ListRequestMessage&);

  ListOptions get_list_options() const;

 protected:
  // measure the size and header size
  void update_static_header();
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);
};

}  // namespace dsa

#endif  // DSA_SDK_LIST_REQUEST_MESSAGE_H_

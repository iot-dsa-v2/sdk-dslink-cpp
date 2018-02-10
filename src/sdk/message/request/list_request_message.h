#ifndef DSA_SDK_LIST_REQUEST_MESSAGE_H
#define DSA_SDK_LIST_REQUEST_MESSAGE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <message/message_options.h>
#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class ListRequestMessage final : public RequestMessage {
 public:
  ListRequestMessage(const uint8_t* data, size_t size);
  ListRequestMessage();
  ListRequestMessage(const ListRequestMessage&);

 protected:
  // measure the size and header size
  void update_static_header() final;
  void print_headers(std::ostream &os) const final;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const final;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);
public:
  ListOptions get_list_options() const;
  void set_list_option(const ListOptions& option);
};

typedef ref_<const ListRequestMessage> ListRequestMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_LIST_REQUEST_MESSAGE_H

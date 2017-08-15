#ifndef DSA_SDK_LIST_RESPONSE_MESSAGE_H_
#define DSA_SDK_LIST_RESPONSE_MESSAGE_H_

#include "../../util/buffer.h"
#include "../../variant/variant.h"
#include "../base_message.h"

namespace dsa {

class ListResponseMessage : public ResponseMessage {
 public:
  ListResponseMessage(const uint8_t* begin, const uint8_t* end);
  ListResponseMessage();
  ListResponseMessage(const ListResponseMessage&);

 protected:
  // measure the size and header size
  void update_static_header();
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_headers(const uint8_t* data,
                             size_t size) throw(const MessageParsingError&);

  std::unique_ptr<DynamicStringHeader> base_path;

  std::unique_ptr<VariantMap> _parsed_map;

 public:
  const std::string& get_base_path() const;
  void set_base_path(const std::string& value);

  const VariantMap& get_map();
};

}  // namespace dsa

#endif  // DSA_SDK_LIST_RESPONSE_MESSAGE_H_

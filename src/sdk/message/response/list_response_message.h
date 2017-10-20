#ifndef DSA_SDK_LIST_RESPONSE_MESSAGE_H_
#define DSA_SDK_LIST_RESPONSE_MESSAGE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>

#include "../../util/buffer.h"
#include "../base_message.h"

namespace dsa {

class VarMap;

class ListResponseMessage : public ResponseMessage {
 public:
  ListResponseMessage(const uint8_t* data, size_t size);
  ListResponseMessage();
  ListResponseMessage(const ListResponseMessage&);

  ~ListResponseMessage();

 protected:
  // measure the size and header size
  void update_static_header();
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);

  std::unique_ptr<DynamicStringHeader> base_path;

  std::unordered_map<std::string, BytesRef> _raw_map;

  void parse();

 public:
  const std::string& get_base_path() const;
  void set_base_path(const std::string& value);

  std::unordered_map<std::string, BytesRef>& get_map() { return _raw_map; };
  const std::unordered_map<std::string, BytesRef>& get_map() const { return _raw_map; };

  ref_<VarMap> get_parsed_map() const;
};

typedef ref_<const ListResponseMessage> ListResponseMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_LIST_RESPONSE_MESSAGE_H_

#ifndef DSA_SDK_LIST_RESPONSE_MESSAGE_H_
#define DSA_SDK_LIST_RESPONSE_MESSAGE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>

#include "util/buffer.h"
#include "../base_message.h"
#include "variant/variant.h"

namespace dsa {

class VarMap;


class ListResponseMessage final : public ResponseMessage {
 public:
  ListResponseMessage(const uint8_t* data, size_t size);
  ListResponseMessage();
  ListResponseMessage(const ListResponseMessage&);

  ~ListResponseMessage();

 protected:
  // measure the size and header size
  void update_static_header() final;
  void print_headers(std::ostream &os) const final;
  // write dynamic header and body
  void write_dynamic_data(uint8_t* data) const final;
  void parse_dynamic_data(const uint8_t* data, size_t dynamic_header_size,
                          size_t body_size) throw(const MessageParsingError&);

  std::unique_ptr<DynamicStringHeader> pub_path;
  std::unique_ptr<DynamicBoolHeader> refreshed;

  std::unordered_map<string_, VarBytesRef> _raw_map;

  void parse();

 public:
  const string_& get_pub_path() const;
  void set_pub_path(const string_& value);

  const bool get_refreshed() const;
  void set_refreshed(bool value);

  std::unordered_map<string_, VarBytesRef>& get_map() { return _raw_map; };
  const std::unordered_map<string_, VarBytesRef>& get_map() const {
    return _raw_map;
  };

  ref_<VarMap> get_parsed_map() const;
};

typedef ref_<const ListResponseMessage> ListResponseMessageCRef;

}  // namespace dsa

#endif  // DSA_SDK_LIST_RESPONSE_MESSAGE_H_

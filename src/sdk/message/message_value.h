#ifndef DSA_SDK_MESSAGE_VALUE_H_
#define DSA_SDK_MESSAGE_VALUE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../variant/variant.h"

namespace dsa {

class MessageValue {
 public:
  enum : size_t { MAX_META_SIZE = 0x7FFF };

 public:
  Var meta;
  Var value;

  MessageValue() = default;

  explicit MessageValue(const Var& value);
  MessageValue(Var&& value);

  MessageValue(const uint8_t* data, size_t size);

  MessageValue(Var&& value, const std::string& ts);

  void parse(const uint8_t* data, size_t size);

  BytesRef to_msgpack() const;

  bool is_empty() const { return meta.is_null() && value.is_null(); }
  bool has_value() const { return !(value.is_null()); }
};

}  // namespace dsa

#endif  // DSA_SDK_MESSAGE_VALUE_H_

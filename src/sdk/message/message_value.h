#ifndef DSA_SDK_MESSAGE_VALUE_H_
#define DSA_SDK_MESSAGE_VALUE_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "../variant/variant.h"

namespace dsa {

class Message;

class MessageValue {
 public:
  Var meta;
  Var value;

  MessageValue() = default;

  explicit MessageValue(const Var& value);
  MessageValue(Var&& value);

  MessageValue(const Message* message);

  MessageValue(Var&& value, const string_& ts);

  void parse(const Message* message);
  template <class MessageClass>
  // if message is paged return last page, otherwise return nullptr
  MessageClass* write(MessageClass* message, int32_t sequence_id = 0) const;

  bool is_empty() const { return meta.is_null() && value.is_null(); }
  bool has_value() const { return !(value.is_null()); }
};

}  // namespace dsa

#endif  // DSA_SDK_MESSAGE_VALUE_H_

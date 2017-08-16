#ifndef DSA_SDK_MESSAGE_VALUE_H_
#define DSA_SDK_MESSAGE_VALUE_H_

#include "../variant/variant.h"

namespace dsa {

class MessageValue {
 public:
  Variant meta;
  Variant value;

  MessageValue(const uint8_t* data, size_t size);

  MessageValue(Variant &value);
  MessageValue(Variant &value, const std::string &ts);
};

}  // namespace dsa

#endif  // DSA_SDK_MESSAGE_VALUE_H_

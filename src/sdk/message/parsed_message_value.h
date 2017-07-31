#ifndef DSA_SDK_PARSED_MESSAGE_VALUE_H_
#define DSA_SDK_PARSED_MESSAGE_VALUE_H_

#include "../variant/variant.h"

namespace dsa {

class ParsedMessageValue {
 public:
  std::unique_ptr<Variant> meta;
  std::unique_ptr<Variant> value;

  ParsedMessageValue(const uint8_t* data, size_t size);
};

}  // namespace dsa

#endif  // DSA_SDK_PARSED_MESSAGE_VALUE_H_

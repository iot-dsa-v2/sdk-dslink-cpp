#ifndef DSA_SDK_MESSAGE_EXCEPTION_H
#define DSA_SDK_MESSAGE_EXCEPTION_H

#include <string>

namespace dsa {

class MessageParsingError : public std::runtime_error {
public:
  explicit MessageParsingError(const std::string& _Message)
      : std::runtime_error(_Message.c_str()) {  // construct from message string
  }
  explicit MessageParsingError(const char* _Message)
      : std::runtime_error(_Message) {  // construct from message string
  }
};
}  // namespace dsa

#endif  // DSA_SDK_MESSAGE_EXCEPTION_H

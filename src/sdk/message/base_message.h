#ifndef DSA_SDK_ABSTRACT_MESSAGE_H_
#define DSA_SDK_ABSTRACT_MESSAGE_H_

#include "static_header.h"

namespace dsa {

class Message {
  StaticHeader staticHeader;
};

class RequestMessage : public Message {

};

class ResponseMessage : public Message {

};

}  // namespace dsa

#endif  // DSA_SDK_ABSTRACT_MESSAGE_H_
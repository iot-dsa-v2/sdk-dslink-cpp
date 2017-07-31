#ifndef DSA_SDK_MESSAGE_MESSAGE_FACTORY_H_
#define DSA_SDK_MESSAGE_MESSAGE_FACTORY_H_

#include "base_message.h"
#include "util/buffer.h"
#include "util/exception.h"

namespace dsa {

Message* parse_message(const SharedBuffer& buffer) throw(
    const MessageParsingError&);

}  // namespace dsa

#endif  // DSA_SDK_MESSAGE_MESSAGE_FACTORY_H_

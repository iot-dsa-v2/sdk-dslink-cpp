#ifndef DSA_SDK_MESSAGE_MESSAGE_FACTORY_H_
#define DSA_SDK_MESSAGE_MESSAGE_FACTORY_H_

#include "../util/buffer.h"
#include "base_message.h"

namespace dsa {

Message* parseMessage(const SharedBuffer& buffer) throw(
    const std::runtime_error&);

}  // namespace dsa

#endif  // DSA_SDK_MESSAGE_MESSAGE_FACTORY_H_

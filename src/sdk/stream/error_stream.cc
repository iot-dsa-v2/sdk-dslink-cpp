#include "dsa_common.h"

#include "error_stream.h"

namespace dsa {
ErrorStream::ErrorStream(int32_t rid, MessageType type, MessageStatus status)
    : MessageStream(rid), _message(new ErrorMessage(type, status)) {
}

size_t ErrorStream::peek_next_message_size(size_t available) {
  return _message->size();
}

MessageCRef ErrorStream::get_next_message(AckCallback& callback) {
  return _message;
}
}

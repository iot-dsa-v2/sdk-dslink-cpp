#include "dsa_common.h"

#include "simple_stream.h"

namespace dsa {
SimpleStream::SimpleStream(int32_t rid, MessageType type, MessageStatus status)
    : MessageStream(rid), _message(new ErrorMessage(type, status)) {}
SimpleStream::SimpleStream(int32_t rid, MessageRef&& msg)
    : MessageStream(rid), _message(std::move(msg)) {}

size_t SimpleStream::peek_next_message_size(size_t available, int64_t time) {
  return _message->size();
}

MessageCRef SimpleStream::get_next_message(AckCallback& callback) {
  return _message;
}
}

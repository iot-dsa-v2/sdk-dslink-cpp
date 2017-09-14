#include "dsa_common.h"

#include "error_stream.h"

namespace dsa {
ErrorStream::ErrorStream(int32_t rid, MessageType type, MessageStatus status)
: MessageStream(rid),_message(type, status){
  // keep the message so it won't be deleted by ref count
  intrusive_ptr_add_ref(&_message);
}

size_t ErrorStream::peek_next_message_size(size_t available) {
  return _message.size();
}

MessageCRef ErrorStream::get_next_message(int32_t ack_id) {
  return _message.get_ref();
}

}

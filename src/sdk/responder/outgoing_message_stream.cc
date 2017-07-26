#include "outgoing_message_stream.h"

#include "network/session.h"

namespace dsa {

OutgoingMessageStream::OutgoingMessageStream(const std::shared_ptr<Session> &session, uint8_t qos, unsigned int id)
    : _qos(qos), _id(id), _strand(*session->_strand) {
  _set_ready = [=]() {
    session->add_ready_stream(_id);
  };
}

void OutgoingMessageStream::new_value(ValueUpdate &new_value) {
  std::lock_guard<std::mutex> lock(_key);
  _message_queue.push_front(new_value);
}

//template <class _MsgType>
//void OutgoingMessageStream::_handle_new_message(ValueUpdate new_value) {
//
//}

}  // namespace dsa
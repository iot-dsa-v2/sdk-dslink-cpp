#include "dsa_common.h"

#include "core/session.h"
#include "outgoing_message_stream.h"

namespace dsa {

OutgoingMessageStream::OutgoingMessageStream(intrusive_ptr_<Session> session,
                                             _stream_container container,
                                             size_t id,
                                             uint32_t rid)
    : MessageStream(session, container, rid, id) {}

/////////////////////////////
// SubscribeMessageStream
/////////////////////////////
SubscribeMessageStream::SubscribeMessageStream(intrusive_ptr_<Session> session,
                                               _stream_container container,
                                               SubscribeOptions config,
                                               size_t id,
                                               uint32_t rid)
    : OutgoingMessageStream(session, container, id, rid), _config(config) {
  _set_ready = [=]() {
    session->responder.set_ready_stream(get_info());
  };
}

void SubscribeMessageStream::new_message(
    const SubscribeResponseMessage &new_message) {
  { _message_queue.push_front(new_message); }
  _set_ready();
}

size_t SubscribeMessageStream::get_next_message_size() {
  return _message_queue.back().size();
}

const Message &SubscribeMessageStream::get_next_message() {
  auto message = _message_queue.back();
  _message_queue.pop_back();
  return std::move(message);
}

///////////////////////////////
// InvokeMessageStream
///////////////////////////////
InvokeMessageStream::InvokeMessageStream(intrusive_ptr_<Session> session,
                                         _stream_container container,
                                         InvokeOptions config,
                                         size_t id,
                                         uint32_t rid)
    : OutgoingMessageStream(session, container, id, rid), _config(config) {
  _set_ready = [=]() {
    session->responder.set_ready_stream(get_info());
  };
}

void InvokeMessageStream::new_message(
    const InvokeResponseMessage &new_message) {
  _message_queue.push_front(new_message);
  _set_ready();
}

size_t InvokeMessageStream::get_next_message_size() {
  return _message_queue.back().size();
}

const Message &InvokeMessageStream::get_next_message() {
  auto message = _message_queue.back();
  _message_queue.pop_back();
  return std::move(message);
}

//////////////////////////
// ListMessageStream
//////////////////////////
ListMessageStream::ListMessageStream(intrusive_ptr_<Session> session,
                                     _stream_container container,
                                     ListOptions config,
                                     size_t id,
                                     uint32_t rid)
    : OutgoingMessageStream(session, container, id, rid), _config(config) {
  _set_ready = [=]() {
    session->responder.set_ready_stream(get_info());
  };
}

void ListMessageStream::new_message(const ListResponseMessage &new_message) {
  { _message_queue.push_front(new_message); }
  _set_ready();
}

size_t ListMessageStream::get_next_message_size() {
  return _message_queue.back().size();
}

const Message &ListMessageStream::get_next_message() {
  auto message = _message_queue.back();
  _message_queue.pop_back();
  return std::move(message);
}

//////////////////////////
// SetMessageStream
//////////////////////////
SetMessageStream::SetMessageStream(intrusive_ptr_<Session> session,
                                   _stream_container container,
                                   SetOptions config,
                                   size_t id,
                                   uint32_t rid)
    : OutgoingMessageStream(session, container, id, rid), _config(config) {
  _set_ready = [=]() {
    session->responder.set_ready_stream(get_info());
  };
}

void SetMessageStream::new_message(const SetResponseMessage &new_message) {
  { _message_queue.push_front(new_message); }
  _set_ready();
}

size_t SetMessageStream::get_next_message_size() {
  return _message_queue.back().size();
}

const Message &SetMessageStream::get_next_message() {
  auto message = _message_queue.back();
  _message_queue.pop_back();
  return std::move(message);
}

}  // namespace dsa
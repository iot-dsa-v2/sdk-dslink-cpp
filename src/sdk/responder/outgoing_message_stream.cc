#include "dsa_common.h"

#include "core/session.h"
#include "outgoing_message_stream.h"

namespace dsa {

OutgoingMessageStream::OutgoingMessageStream(intrusive_ptr_<Session> &&session,
                                             uint32_t request_id,
                                             size_t unique_id)
    : MessageStream(std::move(session), request_id, unique_id) {}

/////////////////////////////
// SubscribeMessageStream
/////////////////////////////
SubscribeMessageStream::SubscribeMessageStream(intrusive_ptr_<Session> &&session,
                                               SubscribeOptions &&config,
                                               uint32_t request_id,
                                               size_t unique_id)
    : OutgoingMessageStream(std::move(session), request_id, unique_id), _config(std::move(config)) {}

void SubscribeMessageStream::new_message(const SubscribeResponseMessage &new_message) {
  _message_queue.push_back(new_message);
  _set_ready();
}

size_t SubscribeMessageStream::get_next_message_size() {
  return _message_queue.front().size();
}

const Message &SubscribeMessageStream::get_next_message() {
  auto message = _message_queue.front();
  _message_queue.pop_front();
  return std::move(message);
}

///////////////////////////////
// InvokeMessageStream
///////////////////////////////
InvokeMessageStream::InvokeMessageStream(intrusive_ptr_<Session> &&session,
                                         InvokeOptions &&config,
                                         uint32_t request_id,
                                         size_t unique_id)
    : OutgoingMessageStream(std::move(session), request_id, unique_id), _config(std::move(config)) {}

void InvokeMessageStream::new_message(const InvokeResponseMessage &new_message) {
  _message_queue.push_back(new_message);
  _set_ready();
}

size_t InvokeMessageStream::get_next_message_size() {
  return _message_queue.front().size();
}

const Message &InvokeMessageStream::get_next_message() {
  auto message = _message_queue.front();
  _message_queue.pop_front();
  return std::move(message);
}

//////////////////////////
// ListMessageStream
//////////////////////////
ListMessageStream::ListMessageStream(intrusive_ptr_<Session> &&session,
                                     ListOptions &&config,
                                     uint32_t request_id,
                                     size_t unique_id)
    : OutgoingMessageStream(std::move(session), request_id, unique_id), _config(std::move(config)) {}

void ListMessageStream::new_message(const ListResponseMessage &new_message) {
  { _message_queue.push_back(new_message); }
  _set_ready();
}

size_t ListMessageStream::get_next_message_size() {
  return _message_queue.front().size();
}

const Message &ListMessageStream::get_next_message() {
  auto message = _message_queue.front();
  _message_queue.pop_front();
  return std::move(message);
}

//////////////////////////
// SetMessageStream
//////////////////////////
SetMessageStream::SetMessageStream(intrusive_ptr_<Session> &&session,
                                   SetOptions &&config,
                                   uint32_t request_id,
                                   size_t unique_id)
    : OutgoingMessageStream(std::move(session), request_id, unique_id), _config(std::move(config)) {}

void SetMessageStream::new_message(const SetResponseMessage &new_message) {
  _message_queue.push_back(new_message);
  _set_ready();
}

size_t SetMessageStream::get_next_message_size() {
  return _message_queue.front().size();
}

const Message &SetMessageStream::get_next_message() {
  auto message = _message_queue.front();
  _message_queue.pop_front();
  return std::move(message);
}

ErrorMessageStream::ErrorMessageStream(intrusive_ptr_<Session> &&session,
                                       MessageType type,
                                       MessageStatus status,
                                       uint32_t request_id)
  : OutgoingMessageStream(std::move(session), request_id, 0),
    _error_message(new ErrorMessage(type, status, request_id)) {}

size_t ErrorMessageStream::get_next_message_size() {
  return _error_message->size();
}

const Message &ErrorMessageStream::get_next_message() {
  return *_error_message;
}

}  // namespace dsa
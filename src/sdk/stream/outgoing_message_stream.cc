#include "dsa_common.h"

#include "core/session.h"

namespace dsa {

OutgoingMessageStream::OutgoingMessageStream(ref_<Session> &&session,
                                             uint32_t rid)
    : MessageQueueStream(std::move(session), rid){}

/////////////////////////////
// SubscribeMessageStream
/////////////////////////////
SubscribeMessageStream::SubscribeMessageStream(
    ref_<Session> &&session, SubscribeOptions &&config,
    uint32_t request_id)
    : OutgoingMessageStream(std::move(session), request_id),
      _config(std::move(config)) {}

void SubscribeMessageStream::new_message(
    const SubscribeResponseMessage &new_message) {}



///////////////////////////////
// InvokeMessageStream
///////////////////////////////
InvokeMessageStream::InvokeMessageStream(ref_<Session> &&session,
                                         InvokeOptions &&config,
                                         uint32_t request_id)
    : OutgoingMessageStream(std::move(session), request_id),
      _config(std::move(config)) {}

void InvokeMessageStream::new_message(
    const InvokeResponseMessage &new_message) {}


//////////////////////////
// ListMessageStream
//////////////////////////
ListMessageStream::ListMessageStream(ref_<Session> &&session,
                                     ListOptions &&config, uint32_t request_id)
    : OutgoingMessageStream(std::move(session), request_id),
      _config(std::move(config)) {}

void ListMessageStream::new_message(const ListResponseMessage &new_message) {}


//////////////////////////
// SetMessageStream
//////////////////////////
SetMessageStream::SetMessageStream(ref_<Session> &&session,
                                   SetOptions &&config, uint32_t request_id)
    : OutgoingMessageStream(std::move(session), request_id),
      _config(std::move(config)) {}


ErrorMessageStream::ErrorMessageStream(ref_<Session> &&session,
                                       MessageType type, MessageStatus status,
                                       uint32_t request_id)
    : OutgoingMessageStream(std::move(session), request_id),
      _error_message(new ErrorMessage(type, status, request_id)) {}


MessageRef ErrorMessageStream::get_next_message() { return _error_message; }

}  // namespace dsa
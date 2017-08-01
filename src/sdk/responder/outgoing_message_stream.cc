#include "dsa_common.h"

#include <boost/thread/locks.hpp>

#include "outgoing_message_stream.h"

namespace dsa {

/////////////////////////////
// SubscribeMessageStream
/////////////////////////////
SubscribeMessageStream::SubscribeMessageStream(const std::shared_ptr<Session> &session,
                                               SubscribeOptions config,
                                               size_t id,
                                               uint32_t rid)
    : OutgoingMessageStream(session, id, rid), _config(config) {
  _set_ready = [=]() {
    session->add_ready_outgoing_stream(_request_id, _unique_id);
  };
}

void SubscribeMessageStream::new_message(const SubscribeResponseMessage &new_message) {
  {
    boost::unique_lock<boost::shared_mutex> lock(_key);
    _message_queue.push_front(new_message);
  }
  _set_ready();
}

size_t SubscribeMessageStream::get_next_message_size() {
  boost::shared_lock<boost::shared_mutex> lock(_key);
  return _message_queue.back().size();
}

const Message &SubscribeMessageStream::get_next_message() {
  boost::unique_lock<boost::shared_mutex> lock(_key);
  auto message = _message_queue.back();
  _message_queue.pop_back();
  return std::move(message);
}

///////////////////////////////
// InvokeMessageStream
///////////////////////////////
InvokeMessageStream::InvokeMessageStream(const std::shared_ptr<Session> &session,
                                         InvokeOptions config,
                                         size_t id,
                                         uint32_t rid)
    : OutgoingMessageStream(session, id, rid), _config(config) {
  _set_ready = [=]() {
    session->add_ready_outgoing_stream(_request_id, _unique_id);
  };
}

void InvokeMessageStream::new_message(const InvokeResponseMessage &new_message) {
  {
    boost::unique_lock<boost::shared_mutex> lock(_key);
    _message_queue.push_front(new_message);
  }
  _set_ready();
}

size_t InvokeMessageStream::get_next_message_size() {
  boost::shared_lock<boost::shared_mutex> lock(_key);
  return _message_queue.back().size();
}

const Message &InvokeMessageStream::get_next_message() {
  boost::unique_lock<boost::shared_mutex> lock(_key);
  auto message = _message_queue.back();
  _message_queue.pop_back();
  return std::move(message);
}

//////////////////////////
// ListMessageStream
//////////////////////////
ListMessageStream::ListMessageStream(const std::shared_ptr<Session> &session,
                                     ListOptions config,
                                     size_t id,
                                     uint32_t rid)
    : OutgoingMessageStream(session, id, rid), _config(config) {
  _set_ready = [=]() {
    session->add_ready_outgoing_stream(_request_id, _unique_id);
  };
}

void ListMessageStream::new_message(const ListResponseMessage &new_message) {
  {
    boost::unique_lock<boost::shared_mutex> lock(_key);
    _message_queue.push_front(new_message);
  }
  _set_ready();
}

size_t ListMessageStream::get_next_message_size() {
  boost::shared_lock<boost::shared_mutex> lock(_key);
  return _message_queue.back().size();
}

const Message &ListMessageStream::get_next_message() {
  boost::unique_lock<boost::shared_mutex> lock(_key);
  auto message = _message_queue.back();
  _message_queue.pop_back();
  return std::move(message);
}

//////////////////////////
// SetMessageStream
//////////////////////////
SetMessageStream::SetMessageStream(const std::shared_ptr<Session> &session,
                                   SetOptions config,
                                   size_t id,
                                   uint32_t rid)
    : OutgoingMessageStream(session, id, rid), _config(config) {
  _set_ready = [=]() {
    session->add_ready_outgoing_stream(_request_id, _unique_id);
  };
}

void SetMessageStream::new_message(const SetResponseMessage &new_message) {
  {
    boost::unique_lock<boost::shared_mutex> lock(_key);
    _message_queue.push_front(new_message);
  }
  _set_ready();
}

size_t SetMessageStream::get_next_message_size() {
  boost::shared_lock<boost::shared_mutex> lock(_key);
  return _message_queue.back().size();
}

const Message &SetMessageStream::get_next_message() {
  boost::unique_lock<boost::shared_mutex> lock(_key);
  auto message = _message_queue.back();
  _message_queue.pop_back();
  return std::move(message);
}

}  // namespace dsa
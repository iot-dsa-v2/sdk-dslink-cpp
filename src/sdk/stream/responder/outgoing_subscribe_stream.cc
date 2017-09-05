#include "dsa_common.h"

#include "outgoing_subscribe_stream.h"

#include "util/date_time.h"

namespace dsa {

OutgoingSubscribeStream::OutgoingSubscribeStream(ref_<Session> &&session,
                                                 const Path &path,
                                                 uint32_t rid,
                                                 SubscribeOptions &&options)
    : MessageQueueStream(std::move(session), path, rid),
      _options(std::move(options)) {}

void OutgoingSubscribeStream::close_impl() {
  if (_callback != nullptr) {
    _callback(*this);
    _callback = nullptr;
  };
}
void OutgoingSubscribeStream::on_update(Callback &&callback) {
  _callback = callback;
}

void OutgoingSubscribeStream::receive_message(MessageCRef &&message) {
  auto request_message = DOWN_CAST<const SubscribeRequestMessage*>(message.get());
  _options = request_message->get_subscribe_options();
  if (_callback != nullptr) {
    _callback(*this);
  }
}

void OutgoingSubscribeStream::send_value(Variant &&value) {
  auto response = make_ref_<SubscribeResponseMessage>();
  response->set_value(MessageValue(std::move(value), DateTime::get_ts()));
  send_message(std::move(response));
}
void OutgoingSubscribeStream::send_value(MessageValue &&value) {
  auto response = make_ref_<SubscribeResponseMessage>();
  response->set_value(std::move(value));
  send_message(std::move(response));
}
}

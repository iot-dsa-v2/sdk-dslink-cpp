#include "dsa_common.h"

#include "outgoing_subscribe_stream.h"

#include "util/date_time.h"

namespace dsa {

static SubscribeOptions empty_subscribe_options;

OutgoingSubscribeStream::OutgoingSubscribeStream(ref_<Session> &&session,
                                                 const Path &path, uint32_t rid,
                                                 SubscribeOptions &&options)
    : MessageQueueStream(std::move(session), path, rid),
      _options(std::move(options)) {}

void OutgoingSubscribeStream::close_impl() {
  if (_option_callback != nullptr) {
    std::move(_option_callback)(*this, empty_subscribe_options);
  };
}
void OutgoingSubscribeStream::on_option_change(Callback &&callback) {
  _option_callback = callback;
}

void OutgoingSubscribeStream::receive_message(MessageCRef &&message) {
  auto request_message =
      DOWN_CAST<const SubscribeRequestMessage *>(message.get());
  SubscribeOptions old_options = std::move(_options);
  _options = request_message->get_subscribe_options();
  if (_option_callback != nullptr) {
    _option_callback(*this, old_options);
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

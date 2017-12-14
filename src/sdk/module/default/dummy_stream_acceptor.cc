#include "dsa_common.h"

#include "dummy_stream_acceptor.h"

#include "message/response/invoke_response_message.h"
#include "message/response/list_response_message.h"
#include "message/response/set_response_message.h"
#include "message/response/subscribe_response_message.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_set_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {
void DummyStreamAcceptor::add(ref_<OutgoingSubscribeStream> &&stream) {
  auto response = make_ref_<SubscribeResponseMessage>();
  response->set_status(MessageStatus::NOT_SUPPORTED);
  stream->send_subscribe_response(std::move(response));
}
void DummyStreamAcceptor::add(ref_<OutgoingListStream> &&stream) {
  stream->update_list_status(MessageStatus::NOT_SUPPORTED);
}
void DummyStreamAcceptor::add(ref_<OutgoingInvokeStream> &&stream) {
  stream->close(MessageStatus::NOT_SUPPORTED);
}
void DummyStreamAcceptor::add(ref_<OutgoingSetStream> &&stream) {
  auto response = make_ref_<SetResponseMessage>();
  response->set_status(MessageStatus::NOT_SUPPORTED);
  stream->send_response(std::move(response));
}
}
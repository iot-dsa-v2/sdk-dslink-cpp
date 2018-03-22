#include "dsa_common.h"

#include "dummy_stream_acceptor.h"

#include "message/response/invoke_response_message.h"
#include "message/response/list_response_message.h"
#include "message/response/set_response_message.h"
#include "message/response/subscribe_response_message.h"
#include "responder/node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_set_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {
void DummyStreamAcceptor::add(ref_<OutgoingSubscribeStream> &&stream) {
  auto response = make_ref_<SubscribeResponseMessage>();
  response->set_status(Status::NOT_SUPPORTED);
  stream->send_subscribe_response(std::move(response));
}
void DummyStreamAcceptor::add(ref_<OutgoingListStream> &&stream) {
  stream->update_response_status(Status::NOT_SUPPORTED);
}
void DummyStreamAcceptor::add(ref_<OutgoingInvokeStream> &&stream) {
  stream->close(Status::NOT_SUPPORTED);
}
void DummyStreamAcceptor::add(ref_<OutgoingSetStream> &&stream) {
  stream->close(Status::NOT_SUPPORTED);
}
ref_<NodeModel> DummyStreamAcceptor::get_profile(const string_ &path,
                                                 bool dsa_standard) {
  return ref_<NodeModel>();
}
}
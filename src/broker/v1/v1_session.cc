#include "dsa_common.h"

#include "v1_session.h"

#include "stream/requester/incoming_invoke_stream.h"
#include "stream/requester/incoming_list_stream.h"
#include "stream/requester/incoming_set_stream.h"
#include "stream/requester/incoming_subscribe_stream.h"
#include "util/string.h"
#include "web_server/socket.h"

namespace dsa {
V1Session::V1Session(const LinkStrandRef &strand, const string_ &dsid,
                     std::vector<uint8_t> &&public_key)
    : _strand(strand),
      _dsid(dsid),
      _salt(generate_random_string(8)),
      _public_key(std::move(public_key)) {}
V1Session::~V1Session() = default;

void V1Session::regenerateSalt() { _salt = generate_random_string(8); }

void V1Session::connected(shared_ptr_<Websocket> &&websocket) {}

void V1Session::write_stream(ref_<MessageStream> &&stream) {}
// this stream must be handled first
void V1Session::write_critical_stream(ref_<MessageStream> &&stream) {}

bool V1Session::destroy_resp_stream(int32_t rid) {
  // TODO
  return false;
}
bool V1Session::destroy_req_stream(int32_t rid) {
  // TODO
  return false;
}

// requester
ref_<IncomingSubscribeStream> V1Session::subscribe(
    const string_ &path, IncomingSubscribeStreamCallback &&callback,
    const SubscribeOptions &options) {
  // TODO
  return nullptr;
}

ref_<IncomingListStream> V1Session::list(const string_ &path,
                                         IncomingListStreamCallback &&callback,
                                         const ListOptions &options) {
  // TODO
  return nullptr;
}

ref_<IncomingInvokeStream> V1Session::invoke(
    IncomingInvokeStreamCallback &&callback,
    ref_<const InvokeRequestMessage> &&message) {
  // TODO
  return nullptr;
}

ref_<IncomingSetStream> V1Session::set(
    IncomingSetStreamCallback &&callback,
    ref_<const SetRequestMessage> &&message) {
  // TODO
  return nullptr;
}

bool V1Session::is_connected() const {
  // TODO
  return true;
}
string_ V1Session::map_pub_path(const string_ &path) {
  // TODO
  return path;
}
};  // namespace dsa

#include "dsa_common.h"

#include "incoming_list_stream.h"

#include "message/request/list_request_message.h"
#include "message/response/list_response_message.h"

namespace dsa {

const ListOptions IncomingListStream::default_options;

IncomingListStream::IncomingListStream(ref_<Session>&& session,
                                       const Path& path, uint32_t rid,
                                       Callback&& callback)
    : MessageCacheStream(std::move(session), path),
      _callback(std::move(callback)) {}
void IncomingListStream::receive_message(MessageCRef&& msg) {
  if (msg->type() == MessageType::LIST_RESPONSE) {
    if (_callback != nullptr) {
      _callback(ref_cast_<const ListResponseMessage>(msg), *this);
    }
  }
}

void IncomingListStream::list(const ListOptions &options) {
  auto msg = make_ref_<ListRequestMessage>();
  msg->set_list_option(options);
  msg->set_target_path(path.full_str());
  send_message(std::move(msg));
}
}

#include "dsa_common.h"

#include "remote_root_node.h"

#include "core/session.h"
#include "message/response/list_response_message.h"
#include "stream/requester/incoming_list_stream.h"

namespace dsa {
RemoteRootNode::RemoteRootNode(LinkStrandRef &&strand, ref_<Session> &&session)
    : RemoteNode(std::move(strand), "", std::move(session)) {}
RemoteRootNode::~RemoteRootNode() = default;

void RemoteRootNode::on_session(Session &session,
                                const shared_ptr_<Connection> &connection) {}

// an empty map, default summary for all remote root node
static VarBytesRef default_summary(new VarBytes(Var::new_map().to_msgpack()));
VarBytesRef &RemoteRootNode::get_summary() { return default_summary; }

void RemoteRootNode::on_list(BaseOutgoingListStream &stream,
                             bool first_request) {
  if (first_request) {
    _remote_list_stream = _remote_session->requester.list(_remote_path, [
      this, keep_ref = get_ref()
    ](IncomingListStream &, ref_<const ListResponseMessage> && msg) {
      for (auto &it : msg->get_map()) {
        if (it.first.empty()) return;
        if (it.first[0] == '$' && _override_metas.count(it.first) != 0) {
          // meta is overridden, skip this update
          continue;
        }
        _list_cache.emplace(it);
        _state->update_list_value(it.first, it.second);
      }
    });
  } else {
    for (auto &it : _list_cache) {
      stream.update_list_value(it.first, it.second);
    }
  }
}
}
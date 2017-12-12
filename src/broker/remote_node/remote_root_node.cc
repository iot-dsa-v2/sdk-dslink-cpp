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
  if (!_remote_session->is_connected()) {
    // when link is not connected, send a temp update for the
    stream.update_list_status(MessageStatus::NOT_AVAILABLE);
    for (auto &it : _override_metas) {
      stream.update_list_value(it.first, it.second);
    }
  }
  if (first_request) {
    _remote_list_stream = _remote_session->requester.list(_remote_path, [
      this, keep_ref = get_ref()
    ](IncomingListStream &, ref_<const ListResponseMessage> && msg) {
      if (msg->get_refreshed()) {
        _state->update_list_refreshed();
        _list_cache.clear();
        send_all_override_metas();
      } else if (_list_cache.empty() && !_override_metas.empty()) {
        // send all override meta with the first list response
        send_all_override_metas();
      }
      _state->update_list_status(msg->get_status());

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

void RemoteRootNode::set_override_meta(const string_ &field, Var &&v) {
  auto ref = make_ref_<VarBytes>(std::move(v));
  _override_metas[field] = ref;
  // since root node always has some override metadata
  // list cache can't be empty when there is a on going list stream
  if (!_list_cache.empty()) {
    _list_cache[field] = ref;
    _state->update_list_value(field, ref);
  }
}
void RemoteRootNode::send_all_override_metas() {
  for (auto &it : _override_metas) {
    _list_cache[it.first] = it.second;
    _state->update_list_value(it.first, it.second);
  }
}
}
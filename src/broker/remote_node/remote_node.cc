#include "dsa_common.h"

#include "remote_node.h"

#include "core/session.h"
#include "message/request/invoke_request_message.h"
#include "message/response/list_response_message.h"
#include "stream/requester/incoming_invoke_stream.h"
#include "stream/requester/incoming_list_stream.h"
#include "stream/requester/incoming_subscribe_stream.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "util/string.h"

namespace dsa {

RemoteNode::RemoteNode(LinkStrandRef &&strand, const string_ &remote_path,
                       ref_<Session> &&session)
    : NodeModelBase(std::move(strand)),
      _remote_path(remote_path),
      _remote_session(std::move(session)) {}
RemoteNode::~RemoteNode() = default;

ModelRef RemoteNode::on_demand_create_child(const Path &path) {
  return make_ref_<RemoteNode>(_strand->get_ref(),
                               str_join_path(_remote_path, path.remain_str()),
                               _remote_session->get_ref());
}

void RemoteNode::destroy_impl() {
  if (_remote_list_stream != nullptr) {
    _remote_list_stream->close();
    _remote_list_stream.reset();
  }
  if (_remote_subscribe_stream != nullptr) {
    _remote_subscribe_stream->close();
    _remote_subscribe_stream.reset();
  }
  NodeModelBase::destroy_impl();
}

void RemoteNode::on_subscribe(const SubscribeOptions &options,
                              bool first_request) {
  if (first_request) {
    _remote_subscribe_stream = _remote_session->requester.subscribe(
        _remote_path, [ this, keep_ref = get_ref() ](
                          IncomingSubscribeStream &,
                          ref_<const SubscribeResponseMessage> && msg) {
          set_subscribe_response(std::move(msg));
        },
        options);
  } else {
    _remote_subscribe_stream->subscribe(options);
  }
}
void RemoteNode::on_unsubscribe() {
  if (_remote_subscribe_stream != nullptr) {
    _remote_subscribe_stream->close();
    _remote_subscribe_stream.reset();
  }
  _cached_value.reset();
}

void RemoteNode::on_list(BaseOutgoingListStream &stream, bool first_request) {
  if (!_remote_session->is_connected()) {
    // when link is not connected, send a temp update for the status
    stream.update_list_status(MessageStatus::NOT_AVAILABLE);
  }
  if (first_request) {
    _remote_list_stream = _remote_session->requester.list(_remote_path, [
      this, keep_ref = get_ref()
    ](IncomingListStream &, ref_<const ListResponseMessage> && msg) {
      if (msg->get_refreshed()) {
        _state->update_list_refreshed();
        _list_cache.clear();
      }
      _state->update_list_status(msg->get_status());

      for (auto &it : msg->get_map()) {
        if (it.first.empty()) return;
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
void RemoteNode::on_unlist() {
  if (_remote_subscribe_stream != nullptr) {
    _remote_list_stream->close();
    _remote_list_stream.reset();
  }
  _list_cache.clear();
}

void RemoteNode::invoke(ref_<OutgoingInvokeStream> &&stream,
                        ref_<NodeState> &parent) {
  RemoteInvokeProxy *p = new RemoteInvokeProxy(std::move(stream), get_ref());
  _invoke_streams.emplace(p, p->get_ref());
}
void RemoteNode::remove_invoke(RemoteInvokeProxy *invoke_proxy) {
  if (is_destroyed()) return;
  auto search = _invoke_streams.find(invoke_proxy);
  if (search != _invoke_streams.end()) {
    _invoke_streams.erase(search);
  }
}
void RemoteNode::set(ref_<OutgoingSetStream> &&stream) {
  // TODO
}
}

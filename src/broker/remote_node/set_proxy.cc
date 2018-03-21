#include "dsa_common.h"

#include "set_proxy.h"

#include "core/session.h"
#include "remote_node.h"
#include "requester/requester.h"
#include "stream/requester/incoming_set_stream.h"
#include "stream/responder/outgoing_set_stream.h"
#include "message/response/set_response_message.h"
#include "message/request/set_request_message.h"

namespace dsa {
RemoteSetProxy::RemoteSetProxy() {}
RemoteSetProxy::RemoteSetProxy(ref_<OutgoingSetStream>&& stream,
                                     ref_<RemoteNode> node)
  : _out_stream(std::move(stream)), _node(std::move(node)) {
  // check request message
  _out_stream->on_request([ this, keep_ref = get_ref() ](
    OutgoingSetStream&, ref_<SetRequestMessage> && req_msg) {

    if (req_msg == nullptr) {
      // out stream is closed by requester
      // let destroy_impl close the in stream
      _out_stream.reset();
      destroy();
    } else if (_in_stream == nullptr) {
      // forward the request
      req_msg->set_target_path(_node->_remote_path);
      _in_stream = _node->_remote_session->requester.set(
        [ this, keep_ref = get_ref() ](
          IncomingSetStream& in_stream,
          ref_<const SetResponseMessage> && resp_msg) {
          auto* p_msg = resp_msg.get();
          if (_out_stream != nullptr) {
            _out_stream->send_response(std::move(resp_msg));
          }
          if (in_stream.is_closed()) {
            // a close response is forwarded, both stream will be destroyed by
            // requester and responder
            _in_stream.reset();
            _out_stream.reset();
            destroy();
          }
        },
        std::move(req_msg));
    } else {
      _in_stream->set(std::move(req_msg));
    }
  });
}

void RemoteSetProxy::destroy_impl() {
  if (_in_stream != nullptr) {
    _in_stream->close();
    _in_stream.reset();
  }
  if (_out_stream != nullptr) {
    _out_stream->close(MessageStatus::DISCONNECTED);
    _out_stream.reset();
  }
  if (_node != nullptr) {
    _node->remove_set(this);
    _node.reset();
  }
}
}

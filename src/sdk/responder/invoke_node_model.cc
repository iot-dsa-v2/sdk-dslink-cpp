#include "dsa_common.h"

#include "invoke_node_model.h"

#include "message/request/invoke_request_message.h"
#include "stream/responder/outgoing_invoke_stream.h"

namespace dsa {
InvokeNodeModel::InvokeNodeModel(LinkStrandRef &&strand,
                                 PermissionLevel require_permission)
    : NodeModel(std::move(strand)) {
  invoke_require_permission(require_permission);
};

void InvokeNodeModel::invoke_require_permission(
    PermissionLevel permission_level) {
  if (permission_level >= PermissionLevel::LIST &&
      permission_level <= PermissionLevel::CONFIG) {
    _invoke_require_permission = permission_level;
    update_property("$invokable", Var(to_string(permission_level)));
  }
}

void InvokeNodeModel::invoke(ref_<OutgoingInvokeStream> &&stream,
                             ref_<NodeState> &parent) {
  if (stream->allowed_permission < _invoke_require_permission) {
    auto error_response = make_ref_<InvokeResponseMessage>();
    error_response->set_status(MessageStatus::PERMISSION_DENIED);
    stream->send_response(std::move(error_response));
    return;
  }
  on_invoke(std::move(stream), parent);
}

SimpleInvokeNode::SimpleInvokeNode(LinkStrandRef &&strand, Callback &&callback,
                                   PermissionLevel require_permission)
    : InvokeNodeModel(std::move(strand), require_permission),
      _callback(std::move(callback)) {}

void SimpleInvokeNode::on_invoke(ref_<OutgoingInvokeStream> &&stream,
                                 ref_<NodeState> &parent) {
  stream->on_request(([this](OutgoingInvokeStream &s,
                             ref_<const InvokeRequestMessage> &&message) {
    Var result = _callback(message->get_value());
    auto response = make_ref_<InvokeResponseMessage>();

    if (result.is_int()) {
      // return int means status code
      int64_t rslt_int = result.get_int();
      if (rslt_int >= static_cast<int64_t>(MessageStatus::CLOSED) &&
          rslt_int <= 0xFF) {
        response->set_status(static_cast<MessageStatus>(rslt_int));
      } else {
        response->set_status(MessageStatus::CLOSED);
      }
    } else {
      if (!result.is_null()) {
        response->set_value(result);
      }
      response->set_status(MessageStatus::CLOSED);
    }

    s.send_response(std::move(response));

  }));
}
}

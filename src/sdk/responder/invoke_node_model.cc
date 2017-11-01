#include "dsa_common.h"

#include "invoke_node_model.h"

#include "stream/responder/outgoing_invoke_stream.h"

namespace dsa {
InvokeNodeModel::InvokeNodeModel(LinkStrandRef &&strand,
                                 PermissionLevel require_permission)
    : NodeModel(std::move(strand)) {
  invoke_require_permission(require_permission);
};

void InvokeNodeModel::invoke_require_permission(PermissionLevel permission_level) {
  if (permission_level >= PermissionLevel::LIST && permission_level <= PermissionLevel::CONFIG) {
    _invoke_require_permission = permission_level;
    _metas["$invokable"] = Var(to_string(permission_level));
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
}

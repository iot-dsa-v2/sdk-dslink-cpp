#include "dsa_common.h"

#include "invoke_node_model.h"

#include "message/request/invoke_request_message.h"
#include "module/logger.h"
#include "node_state.h"
#include "stream/responder/outgoing_invoke_stream.h"

namespace dsa {
InvokeNodeModel::InvokeNodeModel(const LinkStrandRef &strand,
                                 PermissionLevel require_permission)
    : NodeModel(strand) {
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
    stream->close(MessageStatus::PERMISSION_DENIED);
    return;
  }
  on_invoke(std::move(stream), parent);
}

SimpleInvokeNode::SimpleInvokeNode(const LinkStrandRef &strand,
                                   SimpleCallback &&callback,
                                   PermissionLevel require_permission)
    : InvokeNodeModel(strand, require_permission),
      _simple_callback(std::move(callback)) {}

SimpleInvokeNode::SimpleInvokeNode(const LinkStrandRef &strand,
                                   FullCallback &&callback,
                                   PermissionLevel require_permission)
    : InvokeNodeModel(strand, require_permission),
      _full_callback(std::move(callback)) {}

void SimpleInvokeNode::set_callback(FullCallback &&callback) {
  if (_full_callback == nullptr) {
    _full_callback = std::move(callback);
  } else {
    LOG_FATAL(__FILENAME__, LOG << "action callback registered twice");
  }
}

void SimpleInvokeNode::on_invoke(ref_<OutgoingInvokeStream> &&stream,
                                 ref_<NodeState> &parent) {
  stream->on_request(([
    this, ref = get_ref(), parent,
    paged_cache = ref_<IncomingPageCache<InvokeRequestMessage>>()
  ](OutgoingInvokeStream & s,
    ref_<const InvokeRequestMessage> && message) mutable {
    if (message == nullptr) {
      if (_full_callback != nullptr) {
        _full_callback(Var(), *this, s, std::move(parent));
      }
      return;  // nullptr is for destroyed callback, no need to handle here
    }
    message = IncomingPageCache<InvokeRequestMessage>::get_first_page(
        paged_cache, std::move(message));
    if (message == nullptr) {
      // paged message is not ready
      return;
    }

    if (_simple_callback != nullptr) {
      Var result = _simple_callback(message->get_value());
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
    } else if (_full_callback != nullptr) {
      // callback handles everything
      _full_callback(message->get_value(), *this, s, std::move(parent));
    } else {
      s.close();
    }
  }));
}
}  // namespace dsa

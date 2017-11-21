#include "dsa_common.h"

#include "model_base.h"

#include "message/response/invoke_response_message.h"
#include "module/logger.h"
#include "node_state.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_set_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"
#include "util/date_time.h"

namespace dsa {

class InvalidNodeModel : public NodeModelBase {
 public:
  InvalidNodeModel() : NodeModelBase(LinkStrandRef()){};
};

ModelRef NodeModelBase::WAITING = ModelRef(new InvalidNodeModel());
ModelRef NodeModelBase::INVALID = ModelRef(new InvalidNodeModel());
ModelRef NodeModelBase::UNAVAILABLE = ModelRef(new InvalidNodeModel());

NodeModelBase::NodeModelBase(LinkStrandRef &&strand)
    : _strand(std::move(strand)) {}

NodeModelBase::~NodeModelBase() = default;

void NodeModelBase::destroy_impl() {
  _cached_value.reset();
  _state.reset();
  _strand.reset();
}

ModelRef NodeModelBase::get_child(const string_ &name) {
  auto child_state = _state->get_child(name, false);
  if (child_state != nullptr) {
    return child_state->get_model();
  }
  // return nullptr
  return ModelRef();
}

ModelRef NodeModelBase::add_child(const string_ &name, ModelRef &&model) {
  if (_state == nullptr) {
    LOG_FATAL(LOG << "NodeModelBase::add_child shouldn't be "
                     "called before initialize() ");
  }
  auto child_state = _state->get_child(name, true);
  if (child_state->get_model() != nullptr) {
    LOG_FATAL(LOG << "NodeModelBase::add_child, child already exists: "
                  << name);
  }
  child_state->set_model(ModelRef(model));
  return std::move(model);
}
void NodeModelBase::subscribe(const SubscribeOptions &options) {
  bool first_request = !_need_subscribe;
  _need_subscribe = true;
  on_subscribe(options, first_request);
}
void NodeModelBase::unsubscribe() {
  if (_need_subscribe) {
    _need_subscribe = false;
    on_unsubscribe();
  }
}

void NodeModelBase::set_value(Var &&value) {
  _cached_value = make_ref_<SubscribeResponseMessage>(std::move(value));
  if (_need_subscribe) {
    _state->new_subscribe_response(copy_ref_(_cached_value));
  }
}
void NodeModelBase::set_value(MessageValue &&value) {
  auto response = make_ref_<SubscribeResponseMessage>();
  response->set_value(std::move(value));
  _cached_value = response;
  if (_need_subscribe) {
    _state->new_subscribe_response(copy_ref_(_cached_value));
  }
}
void NodeModelBase::set_message(SubscribeResponseMessageCRef &&message) {
  _cached_value = std::move(message);
  if (_need_subscribe) {
    _state->new_subscribe_response(copy_ref_(_cached_value));
  }
}

void NodeModelBase::list(BaseOutgoingListStream &stream) {
  bool first_request = !_need_list;
  _need_list = true;
  on_list(stream, first_request);
}
void NodeModelBase::unlist() {
  if (_need_list) {
    _need_list = false;
    on_unlist();
  }
}
static BytesRef blank_bytes;
BytesRef &NodeModelBase::get_summary() {
  LOG_WARN(_strand->logger(), LOG << "::get_summary not implemented");
  return blank_bytes;
}

void NodeModelBase::invoke(ref_<OutgoingInvokeStream> &&stream,
                           ref_<NodeState> &parent) {
  auto response = make_ref_<InvokeResponseMessage>();
  response->set_status(MessageStatus::NOT_SUPPORTED);
  stream->send_response(std::move(response));
}

void NodeModelBase::set(ref_<OutgoingSetStream> &&stream) {
  auto response = make_ref_<SetResponseMessage>();
  response->set_status(MessageStatus::NOT_SUPPORTED);
  stream->send_response(std::move(response));
}

}  // namespace dsa

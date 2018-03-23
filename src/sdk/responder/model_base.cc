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

static_ref_<NodeModelBase> NodeModelBase::WAITING(new InvalidNodeModel());
static_ref_<NodeModelBase> NodeModelBase::INVALID(new InvalidNodeModel());
static_ref_<NodeModelBase> NodeModelBase::UNAVAILABLE(new InvalidNodeModel());

NodeModelBase::NodeModelBase(const LinkStrandRef &strand) : _strand(strand) {}

NodeModelBase::~NodeModelBase() = default;

void NodeModelBase::destroy_impl() {
  _cached_value.reset();
  _state.reset();
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
    LOG_FATAL(__FILENAME__, LOG << "NodeModelBase::add_child shouldn't be "
                                   "called before initialize() ");
  }
  auto child_state = _state->get_child(name, true);
  if (child_state->get_model() != nullptr) {
    LOG_FATAL(
        __FILENAME__,
        LOG << "NodeModelBase::add_child, child already exists: " << name);
  }
  child_state->set_model(ModelRef(model));
  return std::move(model);
}
void NodeModelBase::remove_child(const string_ &name) {
  _state->remove_child(name);
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

MessageValue NodeModelBase::get_cached_value() const {
  if (_cached_value != nullptr) {
    return _cached_value->get_value();
  }
  return MessageValue();
};

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
void NodeModelBase::set_subscribe_response(
    SubscribeResponseMessageCRef &&message) {
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

VarBytesRef NodeModelBase::get_summary() {
  LOG_ERROR(__FILENAME__, LOG << "::get_summary not implemented");
  static static_ref_<VarBytes> blank_bytes(new VarBytes());
  return blank_bytes;
}

void NodeModelBase::invoke(ref_<OutgoingInvokeStream> &&stream,
                           ref_<NodeState> &parent) {
  stream->close(Status::NOT_SUPPORTED);
}

void NodeModelBase::set(ref_<OutgoingSetStream> &&stream) {
  stream->close(Status::NOT_SUPPORTED);
}

}  // namespace dsa

#include "dsa_common.h"

#include "node_model.h"

#include "module/logger.h"
#include "node_state.h"
#include "util/date_time.h"
#include "stream/responder/outgoing_list_stream.h"
#include "stream/responder/outgoing_subscribe_stream.h"

namespace dsa {

class InvalidNodeModel : public NodeModel {
 public:
  InvalidNodeModel() : NodeModel(nullptr){};
};

ref_<NodeModel> NodeModel::WAITING = make_ref_<InvalidNodeModel>();
ref_<NodeModel> NodeModel::INVALID = make_ref_<InvalidNodeModel>();
ref_<NodeModel> NodeModel::UNAVAILABLE = make_ref_<InvalidNodeModel>();

NodeModel::NodeModel(LinkStrandRef &&strand) : _strand(std::move(strand)) {}

NodeModel::~NodeModel() = default;

ref_<NodeModel> NodeModel::get_child(const std::string &name) {
  auto child_state = _state->get_child(name, false);
  if (child_state != nullptr) {
    return child_state->get_model();
  }
  // return nullptr
  return ref_<NodeModel>();
}

ref_<NodeModel> NodeModel::add_child(const std::string &name,
                                     ref_<NodeModel> &&model) {
  auto child_state = _state->get_child(name, true);
  if (child_state->get_model() != nullptr) {
    LOG_FATAL(_strand->logger(), LOG << "NodeModel already exists: " << name);
  }
  child_state->set_model(std::move(model));
}
void NodeModel::subscribe(const SubscribeOptions &options,
                          SubscribeCallback &&callback) {
  if (callback != nullptr) {
    _subscribe_callback = callback;
    on_subscribe(options);
  } else {
    on_subscribe_option_change(options);
  }
}
void NodeModel::unsubscribe() {
  _subscribe_callback = nullptr;
  on_unsubscribe();
}

void NodeModel::set_value(Variant &&value) {
  _cached_value = make_ref_<SubscribeResponseMessage>(std::move(value));
  if (_subscribe_callback) {
    _subscribe_callback(copy_ref_(_cached_value));
  }
}
void NodeModel::set_value(MessageValue &&value) {
  auto response = make_ref_<SubscribeResponseMessage>();
  response->set_value(std::move(value));
  _cached_value = response;
  if (_subscribe_callback) {
    _subscribe_callback(copy_ref_(_cached_value));
  }
}
void NodeModel::set_message(SubscribeResponseMessageCRef &&message) {
  _cached_value = std::move(message);
  if (_subscribe_callback) {
    _subscribe_callback(copy_ref_(_cached_value));
  }
}

void NodeModel::init_list_stream(OutgoingListStream &stream) {

}

}  // namespace dsa
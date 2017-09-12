#include "dsa_common.h"

#include "node_model.h"

#include "node_state.h"

namespace dsa {

const ref_<NodeModel> NodeModel::WAITING_REF;
const ref_<NodeModel> NodeModel::INVALID_REF;
const ref_<NodeModel> NodeModel::UNAVAILIBLE_REF;

NodeModel::~NodeModel() = default;

ref_<NodeModel> NodeModel::get_child(const std::string &name){
  auto child_state = _state->get_child(name, false);
  if (child_state != nullptr) {
    return child_state->get_model();
  }
  // return nullptr
  return ref_<NodeModel>();
}

void NodeModel::subscribe(const SubscribeOptions &options,
                          SubscribeCallback &&callback) {
  _subscribe_callback = std::move(callback);
}

}  // namespace dsa
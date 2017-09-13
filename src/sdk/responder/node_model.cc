#include "dsa_common.h"

#include "node_model.h"

#include "node_state.h"

namespace dsa {

class InvalidNodeModel : public NodeModel {
 public:
  InvalidNodeModel() : NodeModel(nullptr){};
};

// 3 fixed model pointer for special model ref
static InvalidNodeModel waiting_model;
static InvalidNodeModel invalid_model;
static InvalidNodeModel unavailable_model;

NodeModel *NodeModel::WAITING = &waiting_model;
NodeModel *NodeModel::INVALID = &invalid_model;
NodeModel *NodeModel::UNAVAILABLE = &unavailable_model;

NodeModel::NodeModel(LinkStrandRef strand) : _strand(std::move(strand)) {}

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
                                     ref_<NodeModel> model) {
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

}  // namespace dsa
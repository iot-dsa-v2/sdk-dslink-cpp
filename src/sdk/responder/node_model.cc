#include "dsa_common.h"

#include "node_model.h"

namespace dsa {

const ref_<NodeModel> NodeModel::WAITING_REF;
const ref_<NodeModel> NodeModel::INVALID_REF;
const ref_<NodeModel> NodeModel::UNAVAILIBLE_REF;

void NodeModel::subscribe(const SubscribeOptions &options,
                          SubscribeCallback &&callback) {
  _subscribe_callback = std::move(callback);
}

ref_<NodeModel> &NodeModelManager::get_model(const Path &path) {
  throw "not implemented";
}

}  // namespace dsa
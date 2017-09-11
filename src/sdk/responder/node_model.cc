#include "dsa_common.h"

#include "node_model.h"

#include "node_model.h"

namespace dsa {

const ref_<NodeModel> NodeModel::WAITING_REF;
const ref_<NodeModel> NodeModel::INVALID_REF;
const ref_<NodeModel> NodeModel::UNAVAILIBLE_REF;

ref_<NodeModel> &NodeModelManager::get_model(const Path &path) {
  throw "not implemented";
}

}  // namespace dsa
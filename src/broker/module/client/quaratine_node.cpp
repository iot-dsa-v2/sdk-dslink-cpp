#include "dsa_common.h"

#include "quaratine_node.h"

#include "module/stream_acceptor.h"
#include "responder/node_state.h"

namespace dsa {

QuaratineRoot::QuaratineRoot(LinkStrandRef &&strand)
    : RemoteNodeGroup(std::move(strand)) {}

ref_<RemoteRootNode> QuaratineRoot::create_remote_root(const string_ &name,
                                                       Session &session) {
  auto new_root = make_ref_<QuaratineRemoteRoot>(_strand->get_ref(), session);
  add_list_child(name, new_root->get_ref());
  return std::move(new_root);
}

QuaratineRemoteRoot::QuaratineRemoteRoot(LinkStrandRef &&strand,
                                         Session &session)
    : RemoteRootNode(std::move(strand), session) {}

void QuaratineRemoteRoot::initialize() {
  auto profile =
      _strand->stream_acceptor().get_profile("Broker/Quarantine_Client", true);

  if (profile != nullptr) {
    set_override_meta(
        "$is", Var(profile->get_state()->get_path().move_pos(1).remain_str()));

    for (auto &it : profile->get_list_children()) {
      auto child_state = _state->get_child(it.first, true);
      // add profile node only when it doesn't exist already
      if (child_state->get_model() == nullptr) {
        child_state->set_model(it.second->get_ref());
      }
    }
  }
}

}  // namespace dsa

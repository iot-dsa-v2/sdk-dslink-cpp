#include "dsa_common.h"

#include "remote_node_group.h"

namespace dsa {
RemoteNodeGroup::RemoteNodeGroup(const LinkStrandRef &strand)
    : NodeModel(strand) {}
RemoteNodeGroup::~RemoteNodeGroup() = default;

ref_<RemoteRootNode> RemoteNodeGroup::create_remote_root(const string_ &name,
                                                         BaseSession &session) {
  auto new_root = make_ref_<RemoteRootNode>(_strand, session);
  add_list_child(name, new_root->get_ref());
  return std::move(new_root);
}
}
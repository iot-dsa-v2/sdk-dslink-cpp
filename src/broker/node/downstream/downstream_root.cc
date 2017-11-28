#include "dsa_common.h"

#include "downstream_root.h"

#include "../../remote_node/remote_root_node.h"
#include "core/client_info.h"

namespace dsa {
DownstreamRoot::DownstreamRoot(LinkStrandRef &&strand)
    : NodeModel(std::move(strand)) {}

ref_<RemoteRootNode> DownstreamRoot::get_root_for_client(
    const ClientInfo &info) {
  // TODO implement this
  return ref_<RemoteRootNode>();
}
}
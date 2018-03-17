#include "dsa_common.h"

#include "quaratine_node.h"

namespace dsa {
QuaratineRemoteRoot::QuaratineRemoteRoot(LinkStrandRef &&strand,
                                         Session &session)
    : RemoteRootNode(std::move(strand), session) {}
}  // namespace dsa

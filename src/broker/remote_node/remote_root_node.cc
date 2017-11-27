#include "dsa_common.h"

#include "remote_root_node.h"

#include "core/session.h"

namespace dsa {
RemoteRootNode::RemoteRootNode(LinkStrandRef &&strand, ref_<Session> &&session)
    : RemoteNode(std::move(strand), "", std::move(session)) {}
RemoteRootNode::~RemoteRootNode() = default;
}
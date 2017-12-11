#include "dsa_common.h"

#include "remote_root_node.h"

#include "core/session.h"

namespace dsa {
RemoteRootNode::RemoteRootNode(LinkStrandRef &&strand, ref_<Session> &&session)
    : RemoteNode(std::move(strand), "", std::move(session)) {}
RemoteRootNode::~RemoteRootNode() = default;

void RemoteRootNode::on_session(Session &session,
                                const shared_ptr_<Connection> &connection) {}

// an empty map, default summary for all remote root node
static BytesRef default_summary(new RefCountBytes(Var::new_map().to_msgpack()));
BytesRef &RemoteRootNode::get_summary() { return default_summary; }
}
#include "dsa_common.h"

#include "remote_node.h"

#include "core/session.h"

namespace dsa {

RemoteNode::RemoteNode(LinkStrandRef &&strand, const string_ &remote_path,
                       ref_<Session> &&session)
    : NodeModelBase(std::move(strand)),
      _remote_path(remote_path),
      _remote_session(std::move(session)) {}
RemoteNode::~RemoteNode() = default;

bool RemoteNode::periodic_check(size_t ts) {}

ModelRef RemoteNode::on_demand_create_child(const Path &path) {}

void RemoteNode::on_subscribe(const SubscribeOptions &options,
                              bool first_request) {}
void RemoteNode::on_unsubscribe() {}

void RemoteNode::on_list(BaseOutgoingListStream &stream, bool first_request) {}
void RemoteNode::on_unlist() {}

BytesRef &RemoteNode::get_summary() {}

void RemoteNode::invoke(ref_<OutgoingInvokeStream> &&stream,
                        ref_<NodeState> &parent) {}

void RemoteNode::set(ref_<OutgoingSetStream> &&stream) {}
}

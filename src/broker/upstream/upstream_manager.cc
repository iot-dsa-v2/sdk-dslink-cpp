#include "dsa_common.h"

#include "upstream_manager.h"

#include "../node/pub/pub_root.h"
#include "core/client.h"
#include "responder/node_state.h"
#include "stream/responder/outgoing_invoke_stream.h"
#include "upstream_nodes.h"

namespace dsa {
UpstreamManager::UpstreamManager(const LinkStrandRef& strand)
    : _strand(strand) {}
UpstreamManager::~UpstreamManager() = default;

void UpstreamManager::destroy_impl() {}

void UpstreamManager::add_node(NodeModel& sys_node, BrokerPubRoot& pub_root) {
  pub_root.register_standard_profile_function(
      "Broker/Upstream_Connection/Remove",
      CAST_LAMBDA(SimpleInvokeNode::FullCallback)[this, keepref = get_ref()](
          Var && v, SimpleInvokeNode&, OutgoingInvokeStream & stream,
          ref_<NodeState> && parent) {
        auto* upstream = parent->model_cast<UpstreamConnectionNode>();
        if (upstream != nullptr) {
        }
        stream.close(Status::INVALID_PARAMETER);
      });

  _upstream_root.reset(new UpstreamRootNode(_strand));
  _upstream_root->add_list_child(
      "Add", make_ref_<SimpleInvokeNode>(
                 _strand, [ this, keepref = get_ref() ](Var && v)->Var {
                   if (v.is_map()) {

                   }
                   return Var(Status::INVALID_PARAMETER);
                 }));
  sys_node.add_list_child("Upstream", _upstream_root->get_ref());
}
}  // namespace dsa

#include "dsa_common.h"

#include "sys_root.h"

#include "../../broker.h"
#include "core/strand_timer.h"
#include "module/logger.h"
#include "responder/invoke_node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"

namespace dsa {

BrokerSysRoot::BrokerSysRoot(const LinkStrandRef &strand, ref_<DsBroker> &&broker)
    : NodeModel(strand) {
  if (broker->get_master_token() != "") {
    add_list_child(
        "stop",
        make_ref_<SimpleInvokeNode>(
            _strand,
            [broker = std::move(broker)](Var && v, SimpleInvokeNode & node,
                                         OutgoingInvokeStream & stream,
                                         ref_<NodeState> && parent) {
              // Checking Token
              if (v.get_type() == Var::STRING &&
                  broker->get_master_token() == v.get_string()) {
                broker->strand->add_timer(1000, [broker](bool canceled) {
                  LOG_INFO(__FILENAME__, LOG << "DsBroker stopped");
                  broker->destroy();
                  return false;
                });

                stream.close();
                // make sure the close message is sent asap
                stream.make_critical();
              } else {
                stream.close(Status::INVALID_PARAMETER);
              }
            },
            PermissionLevel::CONFIG));
  }
}
}

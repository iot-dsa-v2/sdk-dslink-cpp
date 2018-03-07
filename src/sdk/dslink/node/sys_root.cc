#include <iostream>
#include "dsa_common.h"

#include "sys_root.h"

#include "../link.h"
#include "core/strand_timer.h"
#include "responder/invoke_node_model.h"
#include "stream/responder/outgoing_invoke_stream.h"

namespace dsa {

LinkSysRoot::LinkSysRoot(LinkStrandRef &&strand, DsLink &lnk)
    : NodeModel(std::move(strand)) {
  if (lnk.get_close_token() != "") {
    add_list_child(
        "Stop",
        make_ref_<SimpleInvokeNode>(
            _strand->get_ref(),
            [link = ref_<DsLink>(lnk.get_ref())](
                Var && v, SimpleInvokeNode & node,
                OutgoingInvokeStream & stream, ref_<NodeState> && parent) {
              // Checking Token
              if (v.get_type() == Var::STRING &&
                  link->get_close_token() == v.get_string()) {
                link->strand->add_timer(1000, [link](bool canceled) {
                  link->destroy();
                  return false;
                });
                stream.close();
                // make sure the close message is sent asap
                stream.make_critical();
              } else {
                stream.close(MessageStatus::INVALID_PARAMETER);
              }
            },
            PermissionLevel::CONFIG));
  }
}
LinkSysRoot::~LinkSysRoot() = default;
}  // namespace dsa

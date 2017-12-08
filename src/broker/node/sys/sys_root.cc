#include "dsa_common.h"

#include "sys_root.h"

#include "../../broker.h"
#include "responder/invoke_node_model.h"

namespace dsa {

SysRoot::SysRoot(LinkStrandRef &&strand, ref_<DsBroker> &&broker)
    : NodeModel(std::move(strand)) {
  add_list_child("stop", make_ref_<SimpleInvokeNode>(
                              _strand->get_ref(),
                              [broker = std::move(broker)](Var && v) {
                                broker->destroy();
                                return Var();
                              },
                              PermissionLevel::CONFIG));
}
}

#include "dsa_common.h"

#include "sys_root.h"

#include "../link.h"
#include "responder/invoke_node_model.h"

namespace dsa {

LinkSysRoot::LinkSysRoot(LinkStrandRef &&strand, ref_<DsLink> &&link)
    : NodeModel(std::move(strand)) {
  add_list_child(
      "stop", make_ref_<SimpleInvokeNode>(_strand->get_ref(),
                                          [link = std::move(link)](Var && v) {
                                            link->destroy();
                                            return Var();
                                          },
                                          PermissionLevel::CONFIG));
}
LinkSysRoot::~LinkSysRoot() = default;
}

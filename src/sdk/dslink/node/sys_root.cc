#include <iostream>
#include "dsa_common.h"

#include "sys_root.h"

#include "../link.h"
#include "responder/invoke_node_model.h"

namespace dsa {

LinkSysRoot::LinkSysRoot(LinkStrandRef &&strand, ref_<DsLink> &&link)
    : NodeModel(std::move(strand)) {
  if (link->get_close_token() != "") {
    add_list_child("stop",
                   make_ref_<SimpleInvokeNode>(
                       _strand->get_ref(),
                       [link = std::move(link)](Var && v) {
                         // Checking Token
                         if (v.get_type() == Var::STRING &&
                             link->get_close_token() == v.get_string()) {
                           link->destroy();
                           return Var();
                         } else {
                           return Var(static_cast<int64_t>(
                               MessageStatus::INVALID_PARAMETER));
                         }
                       },
                       PermissionLevel::CONFIG));
  }
}
LinkSysRoot::~LinkSysRoot() = default;
}

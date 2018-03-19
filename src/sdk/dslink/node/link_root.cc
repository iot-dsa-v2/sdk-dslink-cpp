#include "dsa_common.h"

#include "link_root.h"

#include "module/logger.h"
#include "pub_root.h"
#include "responder/profile/pub_root.h"
#include "sys_root.h"

namespace dsa {
LinkRoot::LinkRoot(const LinkStrandRef &strand, DsLink &link)
    : NodeModel(strand) {
  add_list_child("Sys", new LinkSysRoot(_strand->get_ref(), link));
  _pub_node = new PubRoot(_strand->get_ref(), "");
  add_list_child("Pub", _pub_node->get_ref());
}

LinkRoot::~LinkRoot() = default;
void LinkRoot::destroy_impl() {
  _main_node.reset();
  _pub_node.reset();
  NodeModel::destroy_impl();
}

void LinkRoot::set_main(ref_<NodeModelBase> &&main_node) {
  _main_node = add_list_child("Main", std::move(main_node));
}

ref_<NodeModelBase> LinkRoot::add_to_main(const string_ &name,
                                          ref_<NodeModelBase> &&node) {
  return std::move(_main_node->add_list_child(name, std::move(node)));
}

void LinkRoot::remove_from_main(const string_ &name) {
  _main_node->remove_list_child(name);
}

}  // namespace dsa

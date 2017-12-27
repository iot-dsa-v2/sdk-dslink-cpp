#include "dsa_common.h"

#include "link_root.h"

#include "pub_root.h"
#include "sys_root.h"

namespace dsa {
LinkRoot::LinkRoot(LinkStrandRef &&strand, ref_<DsLink> &&link)
    : NodeModel(std::move(strand)) {
  add_list_child("sys", new LinkSysRoot(_strand->get_ref(), std::move(link)));
  add_list_child("pub", new NodeModel(_strand->get_ref()));
}

LinkRoot::~LinkRoot() = default;
void LinkRoot::destroy_impl() {
  // TODO: remove all children
}

void LinkRoot::set_main(ref_<NodeModelBase> &&main_node) {

  _main_node = add_list_child("main", std::move(main_node));
}

ref_<NodeModelBase> LinkRoot::add_to_main(string_ name, ref_<NodeModelBase> &&node) {
  return std::move(_main_node->add_list_child(name,std::move(node)));
}

}

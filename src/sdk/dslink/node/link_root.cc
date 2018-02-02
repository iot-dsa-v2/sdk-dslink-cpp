#include "dsa_common.h"

#include "link_root.h"

#include "module/logger.h"
#include "pub_root.h"
#include "sys_root.h"

namespace dsa {
LinkRoot::LinkRoot(LinkStrandRef &&strand, DsLink &link)
    : NodeModel(std::move(strand)) {
  add_list_child("sys", new LinkSysRoot(_strand->get_ref(), link));
  _pub_node = new NodeModel(_strand->get_ref());
  add_list_child("pub", _pub_node->get_ref());
}

LinkRoot::~LinkRoot() = default;
void LinkRoot::destroy_impl() {
  _main_node.reset();
  _pub_node.reset();
  NodeModel::destroy_impl();
}

void LinkRoot::set_main(ref_<NodeModelBase> &&main_node) {
  _main_node = add_list_child("main", std::move(main_node));
}

ref_<NodeModelBase> LinkRoot::add_to_main(const string_ &name,
                                          ref_<NodeModelBase> &&node) {
  return std::move(_main_node->add_list_child(name, std::move(node)));
}

ref_<NodeModel> LinkRoot::add_to_pub(const string_ &path_str,
                                     ref_<NodeModel> &&node) {
  Path path(path_str);
  if (!path.is_invalid()) {
    LOG_FATAL(LOG << "invalid pub node path: " << path_str);
  }

  NodeModel *pub_parent = _pub_node.get();
  while (!path.is_last()) {
    ModelRef child = pub_parent->get_child(path.current_name());
    if (child == nullptr) {
      pub_parent = static_cast<NodeModel *>(
          pub_parent
              ->add_list_child(path.current_name(),
                               make_ref_<NodeModel>(_strand->get_ref()))
              .get());
    } else {
      pub_parent = dynamic_cast<NodeModel *>(child.get());
      if (pub_parent == nullptr) {
        LOG_FATAL(LOG << "failed to add pub node: " << path_str);
      }
    }
    path = path.next();
  }
  return std::move(
      pub_parent->add_list_child(path.current_name(), std::move(node)));
}
void LinkRoot::remove_from_main(const string_ &name) {
  _main_node->remove_list_child(name);
}

}  // namespace dsa

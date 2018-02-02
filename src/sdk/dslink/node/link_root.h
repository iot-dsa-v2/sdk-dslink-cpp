#ifndef DSA_DSLINK_LINK_ROOT_H
#define DSA_DSLINK_LINK_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {
class DsLink;

class LinkRoot : public NodeModel {
  friend class DsLink;
  ref_<NodeModel> _main_node;
  ref_<NodeModel> _pub_node;

 public:
  LinkRoot(LinkStrandRef &&strand, DsLink &link);
  ~LinkRoot();

  void set_main(ref_<NodeModelBase> &&main_node);
  ref_<NodeModelBase> add_to_main(const string_ &name,
                                  ref_<NodeModelBase> &&node);
  ref_<NodeModel> add_to_pub(const string_ &path, ref_<NodeModel> &&node);
  void remove_from_main(const string_ &name);

 protected:
  void destroy_impl() final;
};
}  // namespace dsa

#endif  // DSA_DSLINK_LINK_ROOT_H

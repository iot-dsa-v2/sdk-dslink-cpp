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

 public:
  LinkRoot(LinkStrandRef &&strand, ref_<DsLink> &&link);
  ~LinkRoot();

  void set_main(ref_<NodeModelBase> &&main_node);

 protected:
  void destroy_impl() final;
};
}

#endif  // DSA_DSLINK_LINK_ROOT_H

#ifndef DSA_DSLINK_SYS_ROOT_H
#define DSA_DSLINK_SYS_ROOT_H

#include "responder/node_model.h"

namespace dsa {
class DsLink;

class LinkSysRoot final : public NodeModel {
  ref_<DsLink> _link;

 public:
  LinkSysRoot(LinkStrandRef &&strand, ref_<DsLink> &&link);
  ~LinkSysRoot() final;
};
}

#endif  // DSA_DSLINK_SYS_ROOT_H

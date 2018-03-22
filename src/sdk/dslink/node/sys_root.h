#ifndef DSA_DSLINK_SYS_ROOT_H
#define DSA_DSLINK_SYS_ROOT_H

#include "responder/node_model.h"

namespace dsa {
class DsLink;

class LinkSysRoot final : public NodeModel {
  ref_<DsLink> _link;

 public:
  LinkSysRoot(const LinkStrandRef &strand, DsLink &link);
  ~LinkSysRoot() final;
};
}  // namespace dsa

#endif  // DSA_DSLINK_SYS_ROOT_H

#ifndef DSA_DSLINK_PUB_ROOT_H
#define DSA_DSLINK_PUB_ROOT_H

#include "responder/node_model.h"

namespace dsa {

class LinkPubRoot final : public NodeModel {
 public:
  LinkPubRoot(LinkStrandRef &&strand);
  ~LinkPubRoot() final;
};
}

#endif  // DSA_DSLINK_PUB_ROOT_H

#ifndef DSA_SDK_QUARATINE_NODE_H
#define DSA_SDK_QUARATINE_NODE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../../remote_node/remote_node_group.h"
#include "../../remote_node/remote_root_node.h"
#include "util/client_info.h"

namespace dsa {

class QuaratineRoot : public RemoteNodeGroup {
 public:
  QuaratineRoot(const LinkStrandRef &strand);
  ref_<RemoteRootNode> create_remote_root(const string_ &name,
                                          Session &session) override;
};

class QuaratineRemoteRoot : public RemoteRootNode {
 public:
  QuaratineRemoteRoot(const LinkStrandRef &strand, Session &session);

 protected:
  void initialize() override;
};
}  // namespace dsa

#endif  // DSA_SDK_QUARATINE_NODE_H

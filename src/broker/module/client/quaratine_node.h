#ifndef DSA_SDK_QUARATINE_NODE_H
#define DSA_SDK_QUARATINE_NODE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../../remote_node/remote_root_node.h"
#include "util/client_info.h"

namespace dsa {

class QuaratineRemoteRoot : public RemoteRootNode {
  QuaratineRemoteRoot(LinkStrandRef &&strand, Session &session);
};
}  // namespace dsa

#endif  // DSA_SDK_QUARATINE_NODE_H

#ifndef DSA_BROKER_DOWNSTREAM_ROOT_H
#define DSA_BROKER_DOWNSTREAM_ROOT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"

namespace dsa {
class RemoteRootNode;
struct ClientInfo;

class DownstreamRoot : public NodeModel {
 public:
  explicit DownstreamRoot(LinkStrandRef &&strand);

  ref_<RemoteRootNode> get_root_for_client(const ClientInfo & info);
};
}

#endif  // DSA_BROKER_DOWNSTREAM_ROOT_H

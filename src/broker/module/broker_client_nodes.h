#ifndef DSA_SDK_CLIENT_NODES_H
#define DSA_SDK_CLIENT_NODES_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"
#include "util/client_info.h"

namespace dsa {
class BrokerClientsRoot : public NodeModel {
 public:
  explicit BrokerClientsRoot(LinkStrandRef&& strand)
      : NodeModel(std::move(strand)){};

 protected:
  void initialize() override;
};

class BrokerClientNode : public NodeModel {
  ClientInfo _client_info;

 public:
  explicit BrokerClientNode(LinkStrandRef&& strand,
                               ref_<NodeModel>&& profile)
      : NodeModel(std::move(strand), std::move(profile)){};

  ClientInfo& get_client_info() { return _client_info; };
};
}  // namespace dsa

#endif  // DSA_SDK_CLIENT_NODES_H

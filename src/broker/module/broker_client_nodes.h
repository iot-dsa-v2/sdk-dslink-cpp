#ifndef DSA_SDK_CLIENT_NODES_H
#define DSA_SDK_CLIENT_NODES_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"
#include "util/client_info.h"

namespace dsa {
class BrokerClientManager;

class BrokerClientsRoot : public NodeModel {
  friend class BrokerClientManager;
  std::unique_ptr<StorageBucket> _storage;

 public:
  explicit BrokerClientsRoot(LinkStrandRef&& strand);

 protected:
  void initialize() override;
};

class BrokerClientNode : public NodeModel {
  ClientInfo _client_info;

 public:
  explicit BrokerClientNode(LinkStrandRef&& strand, ref_<NodeModel>&& profile);

  ClientInfo& get_client_info() { return _client_info; };

 protected:
  void save_extra(VarMap& map) const override;
  void load_extra(VarMap& map) override;
};
}  // namespace dsa

#endif  // DSA_SDK_CLIENT_NODES_H

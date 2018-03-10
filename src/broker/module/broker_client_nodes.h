#ifndef DSA_SDK_CLIENT_NODES_H
#define DSA_SDK_CLIENT_NODES_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "responder/node_model.h"
#include "util/client_info.h"

namespace dsa {
class BrokerClientManager;
class ValueNodeModel;

class BrokerClientNode : public NodeModel {
  ClientInfo _client_info;

  ref_<ValueNodeModel> _group_node;
  ref_<ValueNodeModel> _path_node;
  ref_<ValueNodeModel> _default_token_node;
  ref_<NodeModel> _max_session_node;
  ref_<NodeModel> _current_session_node;

 public:
  BrokerClientNode(LinkStrandRef&& strand, ref_<NodeModel>&& profile,
                   const string_& dsid);
  ~BrokerClientNode() override;

  const ClientInfo& get_client_info() const { return _client_info; };
  void set_client_info(ClientInfo&& info);

 protected:
  void destroy_impl() override;
  void save_extra(VarMap& map) const override;
  void load_extra(VarMap& map) override;
};

class BrokerClientsRoot : public NodeModel {
  friend class BrokerClientManager;
  friend class BrokerClientNode;

  std::unique_ptr<StorageBucket> _storage;

  ref_<BrokerClientManager> _manager;

 public:
  explicit BrokerClientsRoot(LinkStrandRef&& strand,
                             ref_<BrokerClientManager>&& manager);

 protected:
  void initialize() override;
};

}  // namespace dsa

#endif  // DSA_SDK_CLIENT_NODES_H

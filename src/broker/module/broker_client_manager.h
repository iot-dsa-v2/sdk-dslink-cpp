#ifndef DSA_BROKER_SECURITY_MANAGER_H
#define DSA_BROKER_SECURITY_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "module/client_manager.h"
#include "responder/node_model.h"

namespace dsa {
class ModuleLoader;
class DsBroker;
class BrokerPubRoot;

class BrokerLinksRoot : public NodeModel {
 public:
  explicit BrokerLinksRoot(LinkStrandRef&& strand)
      : NodeModel(std::move(strand)){};

  bool save_child(const string_& name) const override { return true; }

 protected:
  void initialize() override;
  void on_load_child(const string_& name, VarMap& map);
};

class BrokerLinkNode : public NodeModel {
 public:
  explicit BrokerLinkNode(LinkStrandRef&& strand)
      : NodeModel(std::move(strand)){};
};

class BrokerClientManager : public ClientManager {
  friend class DsBroker;

 protected:
  LinkStrandRef _strand;

  ref_<BrokerLinksRoot> _links_node;

  void destroy_impl() override;

 public:
  explicit BrokerClientManager(LinkStrandRef& strand) : _strand(strand){};

  void create_node(NodeModel& module_node, BrokerPubRoot& pub_root);

  void get_client(const string_& dsid, const string_& auth_token,
                  ClientInfo::GetClientCallback&& callback) override;
};
}

#endif  // DSA_BROKER_SESSION_MANAGER_H

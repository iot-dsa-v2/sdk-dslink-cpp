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

class BrokerKnownLinksRoot : public NodeModel {
 public:
  explicit BrokerKnownLinksRoot(LinkStrandRef&& strand)
      : NodeModel(std::move(strand)){};

 protected:
  void initialize() override;
};

class BrokerKnownLinkNode : public NodeModel {
 public:
  explicit BrokerKnownLinkNode(LinkStrandRef&& strand,
                               ref_<NodeModel>&& profile)
      : NodeModel(std::move(strand), std::move(profile)){};
};

class BrokerClientManager : public ClientManager {
  friend class DsBroker;

 protected:
  LinkStrandRef _strand;

  ref_<NodeModel> _known_links;
  ref_<NodeModel> _quarantine;

  void destroy_impl() override;

 public:
  explicit BrokerClientManager(LinkStrandRef& strand) : _strand(strand){};

  void create_nodes(NodeModel& module_node, BrokerPubRoot& pub_root);
  ref_<NodeModel>& get_known_links_node() { return _known_links; }
  ref_<NodeModel>& get_quarantine_node() { return _quarantine; };
  void get_client(const string_& dsid, const string_& auth_token,
                  ClientInfo::GetClientCallback&& callback) override;
};
}  // namespace dsa

#endif  // DSA_BROKER_SESSION_MANAGER_H

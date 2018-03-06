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
class BrokerKnownLinksRoot;
class BrokerKnownLinkNode;

class BrokerClientManager : public ClientManager {
  friend class DsBroker;

 protected:
  LinkStrandRef _strand;

  ref_<NodeModel> _known_links;
  ref_<NodeModel> _quarantine;

  std::unordered_map<string_, string_> _path2id;
  void rebuild_path2id();

  bool _allow_all_links = true;
  bool _quarantine_enabled = false;

  void set_allow_all_links(bool value);
  void set_quarantine_enabled(bool value);

  string_ create_downstream_path(const string_ & dsid);

  void destroy_impl() override;

 public:
  explicit BrokerClientManager(LinkStrandRef& strand) : _strand(strand){};

  void create_nodes(NodeModel& module_node, BrokerPubRoot& pub_root);
  ref_<NodeModel>& get_known_links_node() { return _known_links; }
  ref_<NodeModel>& get_quarantine_node() { return _quarantine; };
  void get_client(const string_& dsid, const string_& auth_token,
                  bool is_responder,
                  ClientInfo::GetClientCallback&& callback) override;
};
}  // namespace dsa

#endif  // DSA_BROKER_SESSION_MANAGER_H

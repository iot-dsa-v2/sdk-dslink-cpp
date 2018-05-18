#ifndef DSA_BROKER_SECURITY_MANAGER_H
#define DSA_BROKER_SECURITY_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "broker_client_manager_config.h"
#include "core/link_strand.h"
#include "module/client_manager.h"
#include "responder/node_model.h"

namespace dsa {
class ModuleLoader;
class DsBroker;
class BrokerPubRoot;
class BrokerClientsRoot;
class BrokerClientNode;
class QuaratineRoot;
class TokensRoot;

class BrokerClientManager : public ClientManager {
  friend class DsBroker;

 protected:
  LinkStrandRef _strand;

  ref_<BrokerClientsRoot> _clients_root;
  ref_<QuaratineRoot> _quarantine_root;
  ref_<TokensRoot> _tokens_root;
  ref_<NodeState> _downstream;

  ref_<BrokerClientManagerConfig> _config;

  bool _allow_all_links = true;
  bool _quarantine_enabled = false;

  void set_allow_all_links(bool value);
  void set_quarantine_enabled(bool value);

  string_ create_downstream_path(const string_& dsid);

  void destroy_impl() override;

 public:
  explicit BrokerClientManager(const LinkStrandRef& strand);
  ~BrokerClientManager() override;

  void rebuild_path2id();

  // return error message
  // return "" for no error, return "err" for a generic error
  StatusDetail update_client_path(const string_& dsid, const string_& new_path);

  void create_nodes(NodeModel& module_node, BrokerPubRoot& pub_root);
  ref_<NodeModel> get_clients_root();
  ref_<NodeModel> get_quarantine_root();
  ref_<NodeModel> get_tokens_root();
  void get_client(const string_& id, const string_& auth_token,
                  bool is_responder,
                  ClientInfo::GetClientCallback&& callback) override;
  void remove_clients_from_token(const string_& token_name);
};
}  // namespace dsa

#endif  // DSA_BROKER_SESSION_MANAGER_H

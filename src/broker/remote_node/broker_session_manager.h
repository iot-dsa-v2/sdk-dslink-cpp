#ifndef DSA_BROKER_SESSION_MANAGER_H
#define DSA_BROKER_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../remote_node/broker_client.h"
#include "module/session_manager.h"

namespace dsa {

class DownstreamRoot;

class BrokerSessionManager final : public SessionManager {
  friend class BrokerClient;

  std::unordered_map<string_, ref_<BrokerClient>> _clients;

  LinkStrandRef _strand;

  ref_<NodeStateManager> _state_manager;

  void client_destroyed(BrokerClient &client);

 protected:
  void destroy_impl() final;

 public:
  BrokerSessionManager(const LinkStrandRef &strand, NodeStateManager &state_manager);
  ~BrokerSessionManager() final;
  void get_session(const string_ &dsid, const string_ &auth_token,
                   bool is_responder,
                   Session::GetSessionCallback &&callback) final;

  void get_session_sync(const ClientInfo &client_info, Session::GetSessionCallback &&callback);

  void remove_sessions(const string_ &dsid, const string_ &responder_path);

  ref_<RemoteRootNode> add_responder_root(const string_ &dsid,
                                          const string_ &responder_path,
                                          Session &session);
  void update_responder_root(const string_ &dsid, const string_ &old_path,
                             const string_ &new_path);
};
}  // namespace dsa

#endif  // DSA_BROKER_SESSION_MANAGER_H

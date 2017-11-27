#ifndef DSA_BROKER_SESSION_MANAGER_H
#define DSA_BROKER_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "module/session_manager.h"

namespace dsa {

class BrokerSessionManager;

/// one client (a dsid) can have multiple sessions at same time
/// these sessions are grouped in ClientSessions class
class BrokerClient final : public DestroyableRef<BrokerClient> {
  friend class BrokerSessionManager;

 protected:
  uint64_t _session_id_seed;
  uint64_t _session_id_count = 0;

  ref_<BrokerSessionManager> _manager;

  ClientInfo _info;
  std::unordered_map<string_, ref_<Session>> _sessions;

  void destroy_impl() final;

  string_ get_new_session_id(const string_ &old_id = "");

  void session_destroyed(Session &session);

 public:
  BrokerClient() = default;
  BrokerClient(ref_<BrokerSessionManager> &&manager, const ClientInfo &info);
  const ClientInfo &info() const { return _info; };
  void add_session(LinkStrandRef &strand, const string_ &session_id,
                   Session::GetSessionCallback &&callback);
};

class BrokerSessionManager final : public SessionManager {
  friend class BrokerClient;

  std::unordered_map<string_, ref_<BrokerClient>> _clients;

  LinkStrandRef _strand;

  void client_destroyed(BrokerClient &client);

 protected:
  void destroy_impl() final;

 public:
  explicit BrokerSessionManager(LinkStrandRef strand);
  void get_session(const string_ &dsid, const string_ &auth_token,
                   const string_ &session_id,
                   Session::GetSessionCallback &&callback) final;
};
}

#endif  // DSA_BROKER_SESSION_MANAGER_H

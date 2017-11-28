#ifndef DSA_BROKER_CLIENT_H
#define DSA_BROKER_CLIENT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/session.h"

namespace dsa {

class BrokerSessionManager;
class RemoteRootNode;

/// one client (a dsid) can have multiple sessions at same time
/// these sessions are grouped in ClientSessions class
class BrokerClient final : public DestroyableRef<BrokerClient> {
  friend class BrokerSessionManager;

 protected:
  uint64_t _session_id_seed;
  uint64_t _session_id_count = 0;

  ref_<BrokerSessionManager> _manager;
  ref_<RemoteRootNode> _node;
  ClientInfo _info;

  // for multiple sessions
  std::unordered_map<string_, ref_<Session>> _sessions;
  // for single session
  ref_<Session> _single_session;

  void destroy_impl() final;

  string_ get_new_session_id(const string_ &old_id = "");

  void session_destroyed(Session &session);

 public:
  BrokerClient(ref_<BrokerSessionManager> &&manager, const ClientInfo &info);
  BrokerClient();
  ~BrokerClient();

  const ClientInfo &info() const { return _info; };
  void add_session(LinkStrandRef &strand, const string_ &session_id,
                   Session::GetSessionCallback &&callback);
};
}
#endif  // DSA_BROKER_CLIENT_H

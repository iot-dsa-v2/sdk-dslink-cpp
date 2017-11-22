#ifndef DSA_BROKER_SESSION_MANAGER_H
#define DSA_BROKER_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "module/session_manager.h"

namespace dsa {

class BrokerSessionManager final : public SessionManager {
  std::unordered_map<string_, ref_<ClientSessions>> _clients;

  LinkStrandRef _strand;

 protected:
  void destroy_impl() final;

 public:
  explicit BrokerSessionManager(LinkStrandRef strand);
  void get_session(const string_ &dsid, const string_ &auth_token,
                   const string_ &session_id,
                   ClientSessions::GetSessionCallback &&callback) final;
};
}

#endif  // DSA_BROKER_SESSION_MANAGER_H

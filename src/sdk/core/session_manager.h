#ifndef DSA_SDK_NETWORK_SESSION_MANAGER_H
#define DSA_SDK_NETWORK_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>

#include "session.h"
#include "util/enable_shared.h"

namespace dsa {

class SecurityManager;
class Config;

class SessionManager : public DestroyableRef<SessionManager> {
  std::unordered_map<string_, ClientSessions> _clients;

  LinkStrandRef _strand;

 protected:
  void destroy_impl() final;

 public:
  SessionManager(LinkStrandRef strand);
  void get_session(const string_ &dsid, const string_ &auth_token,
                   const string_ &session_id,
                   ClientSessions::GetSessionCallback &&callback);
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SESSION_MANAGER_H

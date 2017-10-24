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

class SessionManager {

  std::unordered_map<std::string, ClientSessions> _clients;

  LinkStrandRef _strand;

 public:
  SessionManager(LinkStrandRef strand);
  void get_session(const std::string &dsid, const std::string &auth_token,
                   const std::string &session_id,
                   ClientSessions::GetSessionCallback &&callback);



  void destroy();
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SESSION_MANAGER_H

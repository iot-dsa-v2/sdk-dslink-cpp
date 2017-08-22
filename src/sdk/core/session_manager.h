//
// Created by Ben Richards on 7/24/17.
//

#ifndef DSA_SDK_NETWORK_SESSION_MANAGER_H
#define DSA_SDK_NETWORK_SESSION_MANAGER_H

#include <map>

#include "session.h"
#include "util/enable_shared.h"

namespace dsa {

class SecurityManager;
class Config;

class SessionManager {
 public:
  typedef std::function<void(const intrusive_ptr_<Session> &session)>
      GetSessionCallback;

 private:
  std::map<std::string, intrusive_ptr_<Session>> _sessions;

  LinkStrandPtr _strand;

public:
  SessionManager(LinkStrandPtr strand);
  void get_session(const std::string &dsid, const std::string &auth_token,
                   const std::string &session_id,
                   const GetSessionCallback &&callback);

  std::string get_new_session_id();
  void end_all_sessions();
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SESSION_MANAGER_H

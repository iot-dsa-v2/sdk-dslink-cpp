#ifndef DSA_SDK_USER_SESSION_MANAGER_H
#define DSA_SDK_USER_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>
#include "dsa_common.h"
#include "util/client_info.h"

namespace dsa {
class UserSession;
class UserSessionManager {
 private:
  typedef std::unordered_map<ClientInfo, UserSession> SessionTable;

 public:
  void add_session(const string_& session_cookie);
  void remove_session(const string_& session_cookie);
  bool check_session(const string_& session_cookie);
};
}

#endif  // DSA_SDK_USER_SESSION_MANAGER_H

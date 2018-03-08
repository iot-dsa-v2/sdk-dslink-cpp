#ifndef DSA_SDK_USER_SESSION_MANAGER_H
#define DSA_SDK_USER_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>
#include "dsa_common.h"
#include "util/client_info.h"
#include "user_session.h"
#include <memory>


namespace dsa {
class UserSessionManager {
 private:
  std::unordered_map<string_, shared_ptr_<UserSession>> sessions;

 public:
  const string_ SESSION_COOKIE = "DGSESSION";
  const string_ TOKEN_COOKIE = "DGTOKEN";
  const string_ USER_COOKIE = "DGUSER";
  const int64_t SESSION_TIMEOUT = 100000000;


  void add_session(const string_& session_cookie);
  void remove_session(const string_& session_cookie);
  bool check_session(const string_& session_cookie);
};
}

#endif  // DSA_SDK_USER_SESSION_MANAGER_H

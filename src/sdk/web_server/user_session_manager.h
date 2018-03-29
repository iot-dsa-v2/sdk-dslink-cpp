#ifndef DSA_SDK_USER_SESSION_MANAGER_H
#define DSA_SDK_USER_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <memory>
#include <unordered_map>
#include "dsa_common.h"
#include "user_session.h"
#include "util/client_info.h"

namespace dsa {
class UserSessionManager {
 private:
  std::unordered_map<string_, shared_ptr_<UserSession>> sessions;

 public:
  const string_ SESSION_COOKIE = "DGSESSION";
  const string_ TOKEN_COOKIE = "DGTOKEN";
  const string_ USER_COOKIE = "DGUSER";
  const int64_t SESSION_TIMEOUT = 100000000;

  void add_session(string_, string_ = "", string_ = "");
  void update_session_info(const string_&, const string_&, const string_&);
  bool check_session_info(const string_&, const string_&, const string_&);
  void remove_session(const string_&);
  bool check_session(const string_&);
};
}

#endif  // DSA_SDK_USER_SESSION_MANAGER_H

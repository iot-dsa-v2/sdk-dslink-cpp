#ifndef DSA_SDK_LOGIN_MANAGER_H
#define DSA_SDK_LOGIN_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "util/client_info.h"
#include "util/enable_shared.h"

namespace dsa {
class LoginManager : public SharedDestroyable<LoginManager> {
 public:
  // when user doesn't have a valid session
  virtual void check_login(const string_& username, const string_& password,
                           ClientInfo::GetClientCallback&& callback) = 0;
  // when user already have a valid session
  virtual void get_user(const string_& username,
                        ClientInfo::GetClientCallback&& callback) = 0;
};
}

#endif  // DSA_SDK_LOGIN_MANAGER_H

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
  virtual void get_user(
      const string_& username, ClientInfo::GetClientCallback&& callback) = 0;
};
}

#endif  // DSA_SDK_LOGIN_MANAGER_H

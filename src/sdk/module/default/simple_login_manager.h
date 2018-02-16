#ifndef DSA_SDK_SIMPLE_LOGIN_MANAGER_H
#define DSA_SDK_SIMPLE_LOGIN_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "../../web_server/login_manager.h"
#include "util/client_info.h"

namespace dsa {

class SimpleLoginManager final : public LoginManager {
 public:
  explicit SimpleLoginManager() = default;
  void check_login(const string_& username, const string_& password,
                   ClientInfo::GetClientCallback&& callback) override;
  void get_user(const string_& username,
                ClientInfo::GetClientCallback&& callback) override;
};
}
#endif  // DSA_SDK_SIMPLE_LOGIN_MANAGER_H

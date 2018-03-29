#ifndef DSA_SDK_USER_LOGIN_MANAGER_H
#define DSA_SDK_USER_LOGIN_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/io_service.hpp>
#include "dsa_common.h"
#include "login_manager.h"
#include "util/client_info.h"
#include "web_server.h"

namespace dsa {

class UserLoginManager final : public LoginManager {
  WebServer& _web_server;

 public:
  explicit UserLoginManager(WebServer& web_server);
  void check_login(const string_& username, const string_& password,
                   ClientInfo::GetClientCallback&& callback) override;
  void get_user(const string_& username,
                ClientInfo::GetClientCallback&& callback) override;
};
}

#endif  // DSA_SDK_USER_LOGIN_MANAGER_H

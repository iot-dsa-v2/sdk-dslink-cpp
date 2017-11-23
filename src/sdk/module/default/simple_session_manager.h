#ifndef DSA_SDK_MODULE_SIMPLE_SESSION_MANAGER_H
#define DSA_SDK_MODULE_SIMPLE_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>

#include "../session_manager.h"

namespace dsa {

class SecurityManager;
class Config;

class SimpleSessionManager final : public SessionManager {
  std::unordered_map<string_, ref_<ClientSessions>> _clients;

  LinkStrandRef _strand;

 protected:
  void destroy_impl() final;

 public:
  explicit SimpleSessionManager(LinkStrandRef strand);
  void get_session(const string_ &dsid, const string_ &auth_token,
                   const string_ &session_id,
                   ClientSessions::GetSessionCallback &&callback) final;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SIMPLE_SESSION_MANAGER_H

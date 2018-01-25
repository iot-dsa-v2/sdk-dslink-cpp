#ifndef DSA_SDK_MODULE_SIMPLE_SESSION_MANAGER_H
#define DSA_SDK_MODULE_SIMPLE_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>

#include "../session_manager.h"

namespace dsa {

class ClientManager;
class Config;

class SimpleSessionManager final : public SessionManager {
  std::unordered_map<Session*, ref_<Session>> _sessions;

  LinkStrandRef _strand;
  ClientInfo _last_client;

  unsigned int _count_to_check{0};
  void check_destroyed_session();

 protected:
  void destroy_impl() final;

 public:
  explicit SimpleSessionManager(LinkStrandRef strand);
  void get_session(const string_ &dsid, const string_ &auth_token,
                   Session::GetSessionCallback &&callback) final;
  // a lazy way to clean up unused sessions: after creating 100 new sessions,
  // check if any existing sessions are destroyed and release the reference
  unsigned int memory_check_interval{100};
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SIMPLE_SESSION_MANAGER_H

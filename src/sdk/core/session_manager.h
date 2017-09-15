//
// Created by Ben Richards on 7/24/17.
//

#ifndef DSA_SDK_NETWORK_SESSION_MANAGER_H
#define DSA_SDK_NETWORK_SESSION_MANAGER_H

#include <unordered_map>

#include "client_info.h"
#include "session.h"
#include "util/enable_shared.h"

namespace dsa {

class SecurityManager;
class Config;

class ClientSessions {
 private:
  ClientInfo _info;
  std::unordered_map<std::string, ref_<Session>> _sessions;
 public:
  const ClientInfo &info() const { return _info; };
};

class SessionManager {
 public:
  typedef std::function<void(const ref_<Session> &session)> GetSessionCallback;

 private:
  uint64_t _session_id_seed;
  uint64_t _session_id_count = 0;

  std::unordered_map<std::string, ref_<Session>> _sessions;

  LinkStrand *_strand;

 public:
  SessionManager(LinkStrand *strand);
  void get_session(const std::string &dsid, const std::string &auth_token,
                   const std::string &session_id,
                   GetSessionCallback &&callback);

  std::string get_new_session_id();

  void close();
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SESSION_MANAGER_H

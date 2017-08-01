//
// Created by Ben Richards on 7/24/17.
//

#ifndef DSA_SDK_NETWORK_SESSION_MANAGER_H
#define DSA_SDK_NETWORK_SESSION_MANAGER_H

#include <atomic>
#include <map>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "util/enable_shared.h"

namespace dsa {
class Session;

class SessionManager {
 private:
  std::map<std::string, shared_ptr_<Session>> _sessions;
  std::atomic_long _session_count{0};
  boost::shared_mutex _sessions_key;

 public:
  shared_ptr_<Session> get_session(const std::string &dsid,
                                       const std::string &session_id);
  shared_ptr_<Session> create_session(const std::string &dsid);
  std::string get_new_session_id();
  void end_all_sessions();
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SESSION_MANAGER_H

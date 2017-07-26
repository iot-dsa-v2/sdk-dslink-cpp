//
// Created by Ben Richards on 7/24/17.
//

#ifndef DSA_SDK_NETWORK_SESSION_MANAGER_H
#define DSA_SDK_NETWORK_SESSION_MANAGER_H

#include <map>
#include <atomic>
#include <memory>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>

namespace dsa {
class Session;

class SessionManager {
 private:
  std::map<std::string, std::shared_ptr<Session>> _sessions;
  std::atomic_long _session_count{0};
  boost::shared_mutex _sessions_key;

 public:
  std::shared_ptr<Session> get_session(const std::string &session_id);
  std::shared_ptr<Session> create_session();
  std::string get_new_session_id();
  void end_all_sessions();
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SESSION_MANAGER_H

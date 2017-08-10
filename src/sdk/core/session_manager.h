//
// Created by Ben Richards on 7/24/17.
//

#ifndef DSA_SDK_NETWORK_SESSION_MANAGER_H
#define DSA_SDK_NETWORK_SESSION_MANAGER_H

#include <atomic>
#include <map>

#include <boost/asio/strand.hpp>

#include "session.h"
#include "util/enable_shared.h"

namespace dsa {

class SecurityManager;

class SessionManager {
 public:
  typedef std::function<void(const intrusive_ptr_<Session> &session)>
      GetSessionCallback;

 private:
  std::map<std::string, intrusive_ptr_<Session>> _sessions;
  std::atomic_long _session_count{0};

  boost::asio::io_service::strand &_strand;
  SecurityManager &_security_manager;

 public:
  SessionManager(boost::asio::io_service::strand &strand,
                 SecurityManager &security_manager);
  void get_session(const std::string &dsid, const std::string &auth_token, const std::string &session_id,
                   const GetSessionCallback &callback);

  std::string get_new_session_id();
  void end_all_sessions();
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SESSION_MANAGER_H

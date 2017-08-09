//
// Created by Ben Richards on 7/24/17.
//

#ifndef DSA_SDK_NETWORK_SESSION_MANAGER_H
#define DSA_SDK_NETWORK_SESSION_MANAGER_H

#include <atomic>
#include <map>

#include <boost/thread/locks.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/asio/strand.hpp>

#include "util/enable_shared.h"
#include "session.h"

namespace dsa {

class SessionManager {
 private:
  std::map<std::string, intrusive_ptr_<Session>> _sessions;
  std::atomic_long _session_count{0};
  boost::shared_mutex _sessions_key;
  boost::asio::io_service::strand &_strand;

 public:
  SessionManager(boost::asio::io_service::strand &strand) : _strand(strand) {}
  intrusive_ptr_<Session> get_session(const std::string &dsid,
                                       const std::string &session_id);
  intrusive_ptr_<Session> create_session(const std::string &dsid);
  std::string get_new_session_id();
  void end_all_sessions();
};

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SESSION_MANAGER_H

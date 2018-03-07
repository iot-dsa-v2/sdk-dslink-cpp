#include "user_session_manager.h"

namespace dsa {

void UserSessionManager::add_session(const string_ &session_cookie) {
    sessions.emplace(std::make_pair(session_cookie, std::make_shared<UserSession>(session_cookie)));
}

void UserSessionManager::remove_session(const string_ &session_cookie) {
  auto it = sessions.find(session_cookie);
  if (it != sessions.end())
    sessions.erase(it);
}

bool UserSessionManager::check_session(const string_ &session_cookie) {
  auto it = sessions.find(session_cookie);
  if (it != sessions.end()) {
    if ((DateTime::ms_since_epoch() - it->second->get_timeout()) >
        SESSION_TIMEOUT)
      sessions.erase(it);
    else {
      it->second->update_session_timeout();
      return true;
    }
  }
  return false;
}
}
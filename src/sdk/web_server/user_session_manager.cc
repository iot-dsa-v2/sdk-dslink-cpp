#include "user_session_manager.h"

namespace dsa {

void UserSessionManager::add_session(string_ session_cookie,
                                     string_ user_cookie,
                                     string_ token_cookie) {
  sessions.emplace(std::make_pair(
      session_cookie, std::make_shared<UserSession>(std::move(session_cookie),
                                                    std::move(user_cookie),
                                                    std::move(token_cookie))));
}

void UserSessionManager::remove_session(const string_ &session_cookie) {
  auto it = sessions.find(session_cookie);
  if (it != sessions.end()) sessions.erase(it);
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
void UserSessionManager::update_session_info(const string_ &session_cookie,
                                             const string_ &user_cookie,
                                             const string_ &token_cookie) {
  auto it = sessions.find(session_cookie);
  if (it != sessions.end()) {
    it->second->set_user_cookie(user_cookie);
    it->second->set_token_cookie(token_cookie);
  }
}
bool UserSessionManager::check_session_info(const string_ &session_cookie,
                                            const string_ &user_cookie,
                                            const string_ &token_cookie) {
  auto it = sessions.find(session_cookie);
  if (it != sessions.end()) {
    if (it->second->get_user_cookie() == user_cookie &&
        it->second->get_token_cookie() == token_cookie)
      return true;
  }
  return false;
}
}
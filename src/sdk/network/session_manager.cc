#include "dsa_common.h"

#include "session_manager.h"

#include "session.h"

namespace dsa {

SessionPtr SessionManager::get_session(const std::string &dsid, const std::string &session_id) {
  boost::shared_lock<boost::shared_mutex> lock(_sessions_key);
  if (_sessions.count(session_id) != 0)
    return _sessions.at(session_id);
  return nullptr;
}

SessionPtr SessionManager::create_session(const std::string &dsid) {
  std::string session_id = get_new_session_id();
  auto session = std::make_shared<Session>(session_id);

  {
    boost::unique_lock <boost::shared_mutex> lock(_sessions_key);
    _sessions[session_id] = session;
  }

  return std::move(session);
}

std::string SessionManager::get_new_session_id() {
  Hash hash("sha256");
  hash.update(*gen_salt(32));
  return std::move(std::to_string(_session_count++) + hash.digest_base64());
}

void SessionManager::end_all_sessions() {
  for (auto& kv : _sessions) {
    if (kv.second != nullptr) {
      kv.second->stop();
      kv.second.reset();
    }
  }
}

}  // namespace dsa
#include "dsa_common.h"

#include "session_manager.h"

#include "session.h"

#include "crypto/hash.h"
#include "crypto/misc.h"

#include "module/security_manager.h"

namespace dsa {

SessionManager::SessionManager(boost::asio::io_service::strand &strand,
                               SecurityManager &security_manager)
    : _strand(strand), _security_manager(security_manager) {}

void SessionManager::get_session(const std::string &dsid,
                                 const std::string &auth_token,
                                 const std::string &session_id,
                                 const GetSessionCallback &&callback) {
  _security_manager.get_client(dsid, auth_token, [
    =, callback = std::move(callback)
  ](const ClientInfo client, bool error) {
    if (error) {
      callback(nullptr);
      return;
    }
    if (_sessions.count(session_id) != 0) {
      callback(_sessions.at(session_id));
      return;
    }
    std::string session_id = get_new_session_id();
    auto session = make_intrusive_<Session>(_strand, session_id);

    _sessions[session_id] = session;

    callback(std::move(session));
  });
}

std::string SessionManager::get_new_session_id() {
  Hash hash("sha256");
  hash.update(gen_salt(32));
  return std::move(std::to_string(_session_count++) + hash.digest_base64());
}

void SessionManager::end_all_sessions() {
  for (auto &kv : _sessions) {
    if (kv.second != nullptr) {
      kv.second->close();
      kv.second.reset();
    }
  }
}

}  // namespace dsa
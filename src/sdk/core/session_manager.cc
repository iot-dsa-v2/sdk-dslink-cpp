#include "dsa_common.h"

#include "session_manager.h"

#include "server.h"
#include "crypto/hash.h"
#include "crypto/misc.h"

namespace dsa {
SessionManager::SessionManager(LinkStrandPtr & strand)
    : _strand(strand){}

void SessionManager::get_session(const std::string &dsid,
                                 const std::string &auth_token,
                                 const std::string &session_id,
                                 GetSessionCallback &&callback) {
  _strand->security_manager().get_client(dsid, auth_token, [
    =, callback = std::move(callback)
  ](const ClientInfo client, bool error) mutable {
    if (error) {
      callback(nullptr);
      return;
    }
    if (_sessions.count(session_id) != 0) {
      callback(_sessions.at(session_id));
      return;
    }
    std::string sid = get_new_session_id();
    auto session = make_intrusive_<Session>(_strand, sid);

    _sessions[sid] = session;

    callback(session);
  });
}

std::string SessionManager::get_new_session_id() {
  Hash hash("sha256");
  hash.update(gen_salt(32));
  return std::move(hash.digest_base64());
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
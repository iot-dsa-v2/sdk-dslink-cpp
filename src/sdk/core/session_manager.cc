#include "dsa_common.h"

#include "session_manager.h"

#include "crypto/misc.h"

#include "module/security_manager.h"

namespace dsa {

SessionManager::SessionManager(LinkStrandRef strand) : _strand(strand) {}

void SessionManager::get_session(
    const string_ &dsid, const string_ &auth_token,
    const string_ &session_id,
    ClientSessions::GetSessionCallback &&callback) {
  _strand->security_manager().get_client(
      dsid, auth_token, [ =, callback = std::move(callback) ](
                            const ClientInfo client, bool error) mutable {
        if (error) {
          callback(ref_<Session>());  // return nullptr
          return;
        }
        if (_clients.count(dsid) == 0) {
          _clients[dsid] =  ClientSessions(client);
        }
        _clients[dsid].add_session(_strand, session_id, std::move(callback));

      });
}

void SessionManager::destroy() {
  for (auto &kv : _clients) {
    kv.second.destroy();
  }
  _clients.clear();
  _strand.reset();
}
}  // namespace dsa
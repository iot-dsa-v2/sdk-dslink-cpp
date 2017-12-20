#include "dsa_common.h"

#include "simple_session_manager.h"

#include "module/security_manager.h"

namespace dsa {

SimpleSessionManager::SimpleSessionManager(LinkStrandRef strand)
    : _strand(strand) {}

void SimpleSessionManager::get_session(const string_ &dsid,
                                       const string_ &auth_token,
                                       const string_ &session_id,
                                       int32_t last_ack,
                                       Session::GetSessionCallback &&callback) {
  if (memory_check_interval > 0 && _count_to_check >= memory_check_interval) {
    check_destroyed_session();
  }
  _strand->security_manager().get_client(
      dsid, auth_token, [ =, callback = std::move(callback) ](
                            const ClientInfo client, bool error) mutable {
        if (error) {
          callback(ref_<Session>(), _last_client);  // return nullptr
          return;
        }
        _last_client.dsid = dsid;
        auto search = _sessions.find(session_id);
        if (search != _sessions.end() && search->second->dsid() == dsid) {
          search->second->reconnect(session_id, last_ack);
          callback(search->second, _last_client);
          return;
        } else {
          string_ new_session_id = std::to_string(++_session_seed);
          _sessions[new_session_id] =
              make_ref_<Session>(_strand, client.dsid, new_session_id);
          callback(_sessions[new_session_id], _last_client);
        }
      });
}

void SimpleSessionManager::check_destroyed_session() {
  for (auto it = _sessions.begin(); it != _sessions.end();) {
    if (it->second->is_destroyed()) {
      it = _sessions.erase(it);
    } else {
      ++it;
    }
  }
}

void SimpleSessionManager::destroy_impl() {
  for (auto &kv : _sessions) {
    kv.second->destroy();
  }
  _sessions.clear();
}
}  // namespace dsa
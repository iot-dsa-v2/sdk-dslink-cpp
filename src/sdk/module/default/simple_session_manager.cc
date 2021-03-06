#include "dsa_common.h"

#include "simple_session_manager.h"

#include "module/client_manager.h"

namespace dsa {

SimpleSessionManager::SimpleSessionManager(const LinkStrandRef &strand)
    : _strand(strand) {}

void SimpleSessionManager::get_session(const string_ &dsid,
                                       const string_ &auth_token,
                                       bool is_responder,
                                       Session::GetSessionCallback &&callback) {
  if (memory_check_interval > 0 && _count_to_check >= memory_check_interval) {
    check_destroyed_session();
  }
  _strand->client_manager().get_client(dsid, auth_token, is_responder, [
    =, callback = std::move(callback)
  ](const ClientInfo client, bool error) mutable {
    if (error) {
      callback(ref_<Session>(), _last_client);  // return nullptr
      return;
    }
    _last_client.id = dsid;
    auto session = make_ref_<Session>(_strand, client.id, client.role);
    _sessions[session.get()] = session;

    callback(session, _last_client);

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

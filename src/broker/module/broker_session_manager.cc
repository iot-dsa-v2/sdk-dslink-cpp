#include "dsa_common.h"

#include "broker_session_manager.h"

#include "module/security_manager.h"

namespace dsa {

BrokerSessionManager::BrokerSessionManager(LinkStrandRef strand)
    : _strand(strand) {}

// used by invalid session callback
static ClientInfo dummy_info;

void BrokerSessionManager::get_session(
    const string_ &dsid, const string_ &auth_token, const string_ &session_id,
    ClientSessions::GetSessionCallback &&callback) {
  _strand->security_manager().get_client(
      dsid, auth_token, [ =, callback = std::move(callback) ](
                            const ClientInfo client, bool error) mutable {
        if (error) {
          callback(ref_<Session>(), dummy_info);  // return nullptr
          return;
        }
        if (_clients.count(dsid) == 0) {
          _clients[dsid] = make_ref_<ClientSessions>(client);
        }
        _clients[dsid]->add_session(_strand, session_id, std::move(callback));

      });
}

void BrokerSessionManager::destroy_impl() {
  for (auto &kv : _clients) {
    kv.second->destroy();
  }
  _clients.clear();
}
}  // namespace dsa
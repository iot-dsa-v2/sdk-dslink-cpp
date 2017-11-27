#include "dsa_common.h"

#include "broker_session_manager.h"

#include "crypto/hash.h"
#include "crypto/misc.h"
#include "module/security_manager.h"

namespace dsa {

BrokerClient::BrokerClient(ref_<BrokerSessionManager> &&manager,
                           const ClientInfo &info)
    : _manager(std::move(manager)), _info(info) {
  gen_salt(reinterpret_cast<uint8_t *>(&_session_id_seed), sizeof(uint64_t));
};

void BrokerClient::add_session(LinkStrandRef &strand, const string_ &session_id,
                               Session::GetSessionCallback &&callback) {
  auto search = _sessions.find(session_id);
  if (search != _sessions.end()) {
    callback(search->second, _info);
    return;
  }
  string_ sid = get_new_session_id(session_id);
  auto session = make_ref_<Session>(strand->get_ref(), sid);
  session->set_on_connect([ this, keep_ref = get_ref() ](
      Session & session, const shared_ptr_<Connection> &conn) {

    if (session.is_destroyed()) {
      session_destroyed(session);
    }
  });
  _sessions[sid] = session;

  callback(session, _info);
}

void BrokerClient::session_destroyed(Session &session) {
  auto search = _sessions.find(session.session_id());
  if (search != _sessions.end() && search->second.get() == &session) {
    _sessions.erase(search);
    if (_sessions.empty()) {
      destroy();
    }
  }
}
string_ BrokerClient::get_new_session_id(const string_ &old_session_id) {
  Hash hash;

  std::vector<uint8_t> data(16);
  memcpy(&data[0], &_session_id_seed, sizeof(uint64_t));
  memcpy(&data[8], &_session_id_count, sizeof(uint64_t));
  _session_id_count++;

  hash.update(data);

  string_ result = base64_url_convert(hash.digest_base64());
  if (result != old_session_id && _sessions.find(result) == _sessions.end()) {
    return std::move(result);
  }
  return get_new_session_id(old_session_id);
}

void BrokerClient::destroy_impl() {
  for (auto &kv : _sessions) {
    if (kv.second != nullptr) {
      kv.second->destroy();
    }
  }
  _sessions.clear();
  _manager->client_destroyed(*this);

  // _manager.reset(); is not needed here since the _manager will release the
  // last ref thus delete this object in previous line
}

BrokerSessionManager::BrokerSessionManager(LinkStrandRef strand)
    : _strand(strand) {}

// used by invalid session callback
static ClientInfo dummy_info;

void BrokerSessionManager::get_session(const string_ &dsid,
                                       const string_ &auth_token,
                                       const string_ &session_id,
                                       Session::GetSessionCallback &&callback) {
  _strand->security_manager().get_client(
      dsid, auth_token, [ =, callback = std::move(callback) ](
                            const ClientInfo client, bool error) mutable {
        if (error) {
          callback(ref_<Session>(), dummy_info);  // return nullptr
          return;
        }
        if (_clients.count(dsid) == 0) {
          _clients[dsid] = make_ref_<BrokerClient>(get_ref(), client);
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

void BrokerSessionManager::client_destroyed(BrokerClient &client) {
  auto search = _clients.find(client._info.dsid);
  if (search != _clients.end() && search->second.get() == &client) {
    _clients.erase(search);
  }
}

}  // namespace dsa
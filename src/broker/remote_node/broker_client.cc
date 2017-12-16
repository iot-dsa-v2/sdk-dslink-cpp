#include "dsa_common.h"

#include "broker_client.h"

#include "broker_session_manager.h"
#include "crypto/hash.h"
#include "crypto/misc.h"
#include "module/logger.h"
#include "remote_root_node.h"

namespace dsa {

BrokerClient::BrokerClient(ref_<BrokerSessionManager> &&manager,
                           const ClientInfo &info)
    : _manager(std::move(manager)), _info(info) {
  gen_salt(reinterpret_cast<uint8_t *>(&_session_id_seed), sizeof(uint64_t));
};
BrokerClient::BrokerClient() = default;
BrokerClient::~BrokerClient() = default;

ref_<Session> &BrokerClient::create_single_session(LinkStrandRef &strand) {
  _single_session =
      make_ref_<Session>(strand->get_ref(), _info.dsid, get_new_session_id(""));
  _single_session->set_on_connect([ this, keep_ref = get_ref() ](
      Session & session1, const shared_ptr_<Connection> &conn) {
    if (_node != nullptr) {
      // notify the downstream node about the session change
      _node->on_session(session1, conn);
    }
    if (session1.is_destroyed()) {
      session_destroyed(session1);
    }
  });
  return _single_session;
}
void BrokerClient::add_session(LinkStrandRef &strand, const string_ &session_id,
                               int32_t last_ack,
                               Session::GetSessionCallback &&callback) {
  if (_info.max_session == 1) {
    if (_single_session == nullptr) {
      create_single_session(strand);
    } else if (session_id != _single_session->session_id() &&
               _single_session->reconnect(session_id, last_ack)) {
    } else {
      // when session reconnection expires, it might return false
      // this can only happen on single session
      _single_session->update_session_id(get_new_session_id(session_id));
    }
    callback(_single_session, _info);
  } else {
    auto search = _sessions.find(session_id);
    if (search != _sessions.end()) {
      // reconnect should never return false
      if (!search->second->reconnect(session_id, last_ack)) {
        LOG_ERROR(strand->logger(), LOG << "reconnection failed");
      }
      callback(search->second, _info);
      return;
    }
    string_ sid = get_new_session_id(session_id);
    auto session = make_ref_<Session>(strand->get_ref(), _info.dsid, sid);
    session->set_on_connect([ this, keep_ref = get_ref() ](
        Session & session1, const shared_ptr_<Connection> &conn) {
      if (session1.is_destroyed()) {
        session_destroyed(session1);
      }
    });
    _sessions[sid] = session;

    callback(session, _info);
  }
}

void BrokerClient::session_destroyed(Session &session) {
  if (&session == _single_session.get()) {
    destroy();
    return;
  }
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

  if (_single_session != nullptr) {
    _single_session->destroy();
    _single_session.reset();
  }

  _manager->client_destroyed(*this);
  _manager.reset();
}
}

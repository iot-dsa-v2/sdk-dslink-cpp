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
    : _manager(std::move(manager)),
      _info(info){

      };
BrokerClient::BrokerClient() = default;
BrokerClient::~BrokerClient() = default;

ref_<Session> &BrokerClient::create_single_session(LinkStrandRef &strand) {
  _single_session =
      make_ref_<Session>(strand->get_ref(), _info.id, _info.responder_path);
  _single_session->set_on_connect([ this, keep_ref = get_ref() ](
      Session & session1, const shared_ptr_<Connection> &conn) {
    if (session1.is_destroyed()) {
      session_destroyed(session1);
    }
  });
  return _single_session;
}
void BrokerClient::add_session(LinkStrandRef &strand,
                               Session::GetSessionCallback &&callback) {
  if (_info.max_session == 1) {
    if (_single_session == nullptr) {
      create_single_session(strand);
    }
    callback(_single_session, _info);
  } else {
    auto session = make_ref_<Session>(strand->get_ref(), _info.id);
    session->set_on_connect([ this, keep_ref = get_ref() ](
        Session & session1, const shared_ptr_<Connection> &conn) {
      if (session1.is_destroyed()) {
        session_destroyed(session1);
      }
    });
    _sessions[session.get()] = session;

    callback(session, _info);
  }
}

void BrokerClient::session_destroyed(Session &session) {
  if (&session == _single_session.get()) {
    destroy();
    return;
  }
  auto search = _sessions.find(&session);
  if (search != _sessions.end() && search->second.get() == &session) {
    _sessions.erase(search);
    if (_sessions.empty()) {
      destroy();
    }
  }
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

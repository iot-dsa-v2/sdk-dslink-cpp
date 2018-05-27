#include "dsa_common.h"

#include "v1_session_manager.h"

#include "crypto/hash.h"
#include "crypto/misc.h"
#include "module/client_manager.h"
#include "responder/node_state_manager.h"
#include "util/string.h"
#include "v1_session.h"
#include "web_server/socket.h"

namespace dsa {
V1SessionManager::V1SessionManager(const LinkStrandRef& strand,
                                   NodeStateManager& state_manager,
                                   const string_& dsid)
    : _strand(strand),
      _state_manager(state_manager.get_ref()),
      _dsid(dsid),
      _publick_key_b64(base64_encode(strand->ecdh().get_public_key())),
      _shared_ptr(SharedRef<V1SessionManager>::make(get_ref(), strand)) {}
V1SessionManager::~V1SessionManager() = default;

void V1SessionManager::destroy_impl() {
  for (auto& session : _sessions) {
    session.second->destroy();
  }
  _sessions.clear();
  _state_manager.reset();
}

void V1SessionManager::on_conn(const string_& dsid, const string_& token,
                               const string_& body,
                               std::function<void(const string_&)>&& callback) {
  Var json = Var::from_json(body);
  if (json["isResponder"].is_bool() && json["publicKey"].is_string()) {
    bool is_responder = json["isResponder"].get_bool();
    const string_& remote_publickey = json["publicKey"].get_string();
    auto remove_publickey_binary = base64_decode(remote_publickey);
    {
      // validate remote public key on the dsid
      Hash hash;
      hash.update(remove_publickey_binary);
      if (!str_ends_with(dsid, hash.digest_base64())) {
        callback("");
        return;
      }
    }

    _strand->client_manager().get_client(
        dsid, token, is_responder,
        CAST_LAMBDA(ClientInfo::GetClientCallback)
            [this, keepref = this->get_ref(), callback = std::move(callback),
             dsid, remove_publickey_binary = std::move(remove_publickey_binary),
             json = std::move(json)](const ClientInfo client,
                                     bool error) mutable {
              if (error) {
                callback("");
                return;
              }
              if (!_sessions.count(dsid)) {
                _sessions[dsid] = make_ref_<V1Session>(
                    _strand, dsid, std::move(remove_publickey_binary));
              }
              auto& session = _sessions[dsid];

              Var response = Var({{"dsId", Var(_dsid)},
                                  {"wsUri", Var("/ws")},
                                  {"publicKey", Var(_publick_key_b64)},
                                  {"tempKey", Var(_publick_key_b64)},
                                  {"salt", Var(session->current_salt())},
                                  {"format", Var("json")},
                                  {"version", Var("1.1.2")}});
              callback(response.to_json(1));
            });
  } else {
    // invalid handshake
    callback("");
  }
}
void V1SessionManager::on_ws(shared_ptr_<Websocket>&& ws, const string_& dsid,
                             const string_& auth) {
  if (_sessions.count(dsid)) {
    auto& session = _sessions[dsid];

    if (session->_shared_secret.empty()) {
      session->_shared_secret =
          _strand->ecdh().compute_secret(session->_public_key);
    }
    const uint8_t* unsigned_begin =
        reinterpret_cast<const uint8_t*>(session->_salt.data());
    std::vector<uint8_t> v(unsigned_begin,
                           unsigned_begin + session->_salt.length());
    v.insert(v.end(), session->_shared_secret.begin(),
             session->_shared_secret.end());
    Hash hash;
    hash.update(v);
    if (auth == hash.digest_base64()) {
      session->connected(std::move(ws));
      return;
    }
  }

  ws->destroy();
}
}  // namespace dsa
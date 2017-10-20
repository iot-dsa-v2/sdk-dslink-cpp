#include "dsa_common.h"

#include "session_manager.h"

#include "crypto/hash.h"
#include "crypto/misc.h"
#include "server.h"
#include "module/security_manager.h"

namespace dsa {
SessionManager::SessionManager(LinkStrand *strand) : _strand(strand) {
  gen_salt(reinterpret_cast<uint8_t *>(&_session_id_seed), sizeof(uint64_t));
}

void SessionManager::get_session(const std::string &dsid,
                                 const std::string &auth_token,
                                 const std::string &session_id,
                                 GetSessionCallback &&callback) {
  _strand->security_manager().get_client(
      dsid, auth_token, [ =, callback = std::move(callback) ](
                            const ClientInfo client, bool error) mutable {
        if (error) {
          callback(ref_<Session>());  // return nullptr
          return;
        }
        if (_sessions.count(session_id) != 0) {
          callback(_sessions.at(session_id));
          return;
        }
        std::string sid = get_new_session_id();
        auto session = make_ref_<Session>(_strand->get_ref(), sid);

        _sessions[sid] = session;

        callback(session);
      });
}

std::string SessionManager::get_new_session_id() {
  Hash hash("sha256");

  std::vector<uint8_t> data(16);
  memcpy(&data[0], &_session_id_seed, sizeof(uint64_t));
  memcpy(&data[8], &_session_id_count, sizeof(uint64_t));
  _session_id_count++;

  hash.update(data);
  return std::move(base64_url_convert(hash.digest_base64()));
}

void SessionManager::close() {
  for (auto &kv : _sessions) {
    if (kv.second != nullptr) {
      kv.second->destroy();
      kv.second.reset();
    }
  }
}
}  // namespace dsa
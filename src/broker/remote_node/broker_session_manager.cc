#include "dsa_common.h"

#include "broker_session_manager.h"

#include "../node/downstream/downstream_root.h"
#include "broker_client.h"
#include "module/client_manager.h"
#include "remote_node.h"
#include "remote_root_node.h"

namespace dsa {

BrokerSessionManager::BrokerSessionManager(LinkStrandRef strand,
                                           ref_<DownstreamRoot> downstream_root)
    : _strand(std::move(strand)),
      _downstream_root(std::move(downstream_root)) {}
BrokerSessionManager::~BrokerSessionManager() = default;
// used by invalid session callback
static ClientInfo dummy_info;

void BrokerSessionManager::get_session(const string_ &dsid,
                                       const string_ &auth_token,
                                       bool is_responder,
                                       Session::GetSessionCallback &&callback) {
  _strand->client_manager().get_client(dsid, auth_token, is_responder, [
    =, callback = std::move(callback)
  ](const ClientInfo client_info, bool error) mutable {
    if (error) {
      callback(ref_<Session>(), dummy_info);  // return nullptr
      return;
    }
    if (_clients.count(dsid) == 0) {
      // create the client and add the current session to it
      auto client = make_ref_<BrokerClient>(get_ref(), client_info);
      _clients[dsid] = client;
      client->add_session(_strand, std::move(callback));
      if (client_info.max_session == 1 && !client_info.responder_path.empty()) {
        // init the downstream node after session get connected
        _downstream_root->get_root_for_client(client_info,
                                              *client->_single_session);
      }
    } else {
      _clients[dsid]->add_session(_strand, std::move(callback));
    }
  });
}
void BrokerSessionManager::remove_sessions(const string_ &dsid,
                                           const string_ &responder_path) {
  auto search = _clients.find(dsid);
  if (search != _clients.end()) {
    search->second->destroy();
    _clients.erase(search);
  }
  // TODO remove the downstream node if it's there
}

void BrokerSessionManager::destroy_impl() {
  for (auto &kv : _clients) {
    kv.second->destroy();
  }
  _clients.clear();
  _downstream_root.reset();
}

void BrokerSessionManager::client_destroyed(BrokerClient &client) {
  // return when this is called from destroy_impl
  if (is_destroyed()) return;

  auto search = _clients.find(client._info.id);
  if (search != _clients.end() && search->second.get() == &client) {
    _clients.erase(search);
  }
}

}  // namespace dsa

#include "dsa_common.h"

#include "broker_session_manager.h"

#include "broker_client.h"
#include "module/client_manager.h"
#include "module/logger.h"
#include "remote_node.h"
#include "remote_root_node.h"
#include "responder/node_model.h"

namespace dsa {

BrokerSessionManager::BrokerSessionManager(LinkStrandRef strand,
                                           NodeStateManager &state_manager)
    : _strand(std::move(strand)), _state_manager(state_manager.get_ref()) {}
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
        add_responder_root(client_info.id, client_info.responder_path,
                           *client->_single_session);
      }
    } else {
      _clients[dsid]->add_session(_strand, std::move(callback));
    }
  });
}

void BrokerSessionManager::update_responder_root(const string_ &dsid,
                                                 const string_ &old_path,
                                                 const string_ &new_path) {
  // remove old node
  if (!old_path.empty()) {
    Path path(old_path);
    auto state = _state_manager->check_state(path);
    if (state != nullptr && state->model_cast<RemoteRootNode>() != nullptr) {
      auto *parent_model = state->get_parent()->model_cast<NodeModel>();
      if (parent_model != nullptr) {
        parent_model->remove_list_child(state->get_path().last_name());
      }
    }
  }
  // add new node
  if (!new_path.empty()) {
    auto search = _clients.find(dsid);
    if (search != _clients.end()) {
      auto rslt =
          add_responder_root(dsid, new_path, *search->second->_single_session);
      if (rslt != nullptr) {
        search->second->_info.responder_path = new_path;
      }
    }
  }
}

void BrokerSessionManager::remove_sessions(const string_ &dsid,
                                           const string_ &responder_path) {
  update_responder_root(dsid, responder_path, "");

  auto search = _clients.find(dsid);
  if (search != _clients.end()) {
    search->second->destroy();
    //_clients.erase(search); // already handled in client->destroy()
  }
}

ref_<RemoteRootNode> BrokerSessionManager::add_responder_root(
    const string_ &dsid, const string_ &responder_path, Session &session) {
  Path path(responder_path);
  if (path.is_invalid() || path.data()->names.size() < 2) {
    // downstream name too short
    return ref_<RemoteRootNode>();
  }
  Path parent_path = path.get_parent_path();

  auto parent_state = _state_manager->check_state(parent_path);
  if (parent_state == nullptr ||
      parent_state->model_cast<NodeModel>() == nullptr) {
    LOG_ERROR(__FILENAME__,
              LOG << "failed to add responder, parent node not created: "
                  << responder_path << "  dsid:" << dsid);
    return ref_<RemoteRootNode>();
  }
  auto state = _state_manager->check_state(path);
  if (state != nullptr && state->get_model() != nullptr) {
    LOG_ERROR(__FILENAME__,
              LOG << "failed to add responder, node already exists: "
                  << responder_path << "  dsid:" << dsid);
    return ref_<RemoteRootNode>();
  }

  auto new_root =
      make_ref_<RemoteRootNode>(_strand->get_ref(), session.get_ref());
  new_root->set_override_meta("$$dsid", Var(dsid));
  parent_state->model_cast<NodeModel>()->add_list_child(path.last_name(),
                                                        new_root->get_ref());
  return std::move(new_root);
}

void BrokerSessionManager::destroy_impl() {
  for (auto &kv : _clients) {
    kv.second->destroy();
  }
  _clients.clear();
  _state_manager.reset();
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

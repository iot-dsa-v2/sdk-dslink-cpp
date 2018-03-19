#include "dsa_common.h"

#include "broker_login_manager.h"
#include "responder/node_model.h"

namespace dsa {

BrokerLoginManager::BrokerLoginManager(const LinkStrandRef &strand)
    : _strand(strand) {}
BrokerLoginManager::~BrokerLoginManager() = default;

void BrokerLoginManager::check_login(const string_ &username,
                                     const string_ &password,
                                     ClientInfo::GetClientCallback &&callback) {
  _strand->post([ username, callback = std::move(callback) ]() {
    callback(ClientInfo(username), false);
  });
}

void BrokerLoginManager::get_user(const string_ &username,
                                  ClientInfo::GetClientCallback &&callback) {
  _strand->post([ username, callback = std::move(callback) ]() {
    callback(ClientInfo(username), false);
  });
}
}

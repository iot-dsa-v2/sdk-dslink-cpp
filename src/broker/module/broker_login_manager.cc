#include "dsa_common.h"

#include <dslink.h>
#include "broker_login_manager.h"

namespace dsa {

BrokerLoginManager::BrokerLoginManager(LinkStrandRef strand)
    : _strand(std::move(strand)) {}

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

#include "dsa_common.h"

#include "broker_security_manager.h"

namespace dsa {

BrokerSecurityManager::BrokerSecurityManager(LinkStrandRef strand)
    : _strand(std::move(strand)){};

void BrokerSecurityManager::get_client(const string_& dsid,
                                       const string_& auth_token,
                                       GetClientCallback&& callback) {
  _strand->post([ =, callback = std::move(callback) ]() {
    callback(ClientInfo(dsid, auth_token), false);
  });
}

void BrokerSecurityManager::check_permission(
    const string_& dsid, const string_& permission_token, MessageType method,
    const Path& path, CheckPermissionCallback&& callback) {
  _strand->post([ =, callback = std::move(callback) ]() {
    callback(PermissionLevel::CONFIG);
  });
}
}

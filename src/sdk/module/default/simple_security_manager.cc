#include "dsa_common.h"

#include "simple_security_manager.h"

#include <boost/asio/strand.hpp>

namespace dsa {
void SimpleSecurityManager::get_client(const std::string& dsid,
                                       const std::string& auth_token,
                                       GetClientCallback&& callback) {
  callback(ClientInfo(dsid, auth_token), false);
}

void SimpleSecurityManager::check_permission(
    const std::string& dsid, const std::string& permission_token,
    MessageType method, const Path& path, CheckPermissionCallback&& callback) {
  callback(PermissionLevel::CONFIG);
}

AsyncSimpleSecurityManager::AsyncSimpleSecurityManager(
  LinkStrandRef strand)
    : _strand(strand){};

void AsyncSimpleSecurityManager::get_client(const std::string& dsid,
                                            const std::string& auth_token,
                                            GetClientCallback&& callback) {
  _strand->post([ =, callback = std::move(callback) ]() {
    callback(ClientInfo(dsid, auth_token), false);
  });
  ;
}

void AsyncSimpleSecurityManager::check_permission(
    const std::string& dsid, const std::string& permission_token,
    MessageType method, const Path& path, CheckPermissionCallback&& callback) {
  _strand->post([ =, callback = std::move(callback) ]() {
    callback(PermissionLevel::CONFIG);
  });
}

}  // namespace dsa

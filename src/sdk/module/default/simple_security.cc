#include "dsa_common.h"

#include "simple_security.h"

#include <boost/asio/strand.hpp>

namespace dsa {

SimpleAuthorizer::SimpleAuthorizer(const LinkStrandRef &strand)
    : _strand(strand){};

void SimpleClientManager::get_client(const string_& dsid,
                                     const string_& auth_token,
                                     bool is_responder,
                                     ClientInfo::GetClientCallback&& callback) {
  callback(ClientInfo(dsid), false);
}

AsyncSimpleClientManager::AsyncSimpleClientManager(const LinkStrandRef &strand)
    : _strand(strand){};

void AsyncSimpleClientManager::get_client(
    const string_& dsid, const string_& auth_token, bool is_responder,
    ClientInfo::GetClientCallback&& callback) {
  _strand->post([ dsid, auth_token, callback = std::move(callback) ]() {
    callback(ClientInfo(dsid), false);
  });
}

void SimpleAuthorizer::check_permission(const string_& id, const string_& role,
                                        const string_& permission_token,
                                        MessageType method, const Path& path,
                                        CheckPermissionCallback&& callback) {
  _strand->post([callback = std::move(callback)]() {
    callback(PermissionLevel::CONFIG);
  });
}

}  // namespace dsa

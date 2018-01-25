#include "dsa_common.h"

#include "simple_security.h"

#include <boost/asio/strand.hpp>

namespace dsa {

SimpleAuthorizer::SimpleAuthorizer(LinkStrandRef strand)
    : _strand(std::move(strand)){};

void SimpleClientManager::get_client(const string_& dsid,
                                     const string_& auth_token,
                                     GetClientCallback&& callback) {
  callback(ClientInfo(dsid, auth_token), false);
}

AsyncSimpleClientManager::AsyncSimpleClientManager(LinkStrandRef strand)
    : _strand(std::move(strand)){};

void AsyncSimpleClientManager::get_client(const string_& dsid,
                                            const string_& auth_token,
                                            GetClientCallback&& callback) {
  _strand->post([ =, callback = std::move(callback) ]() {
    callback(ClientInfo(dsid, auth_token), false);
  });
}

void SimpleAuthorizer::check_permission(const string_& dsid,
                                        const string_& permission_token,
                                        MessageType method, const Path& path,
                                        CheckPermissionCallback&& callback) {
  //_strand->post([ =, callback = std::move(callback) ]() {
    callback(PermissionLevel::CONFIG);
  //});
}

}  // namespace dsa

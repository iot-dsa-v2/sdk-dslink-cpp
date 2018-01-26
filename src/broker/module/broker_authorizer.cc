#include "dsa_common.h"

#include "broker_authorizer.h"

namespace dsa {

void BrokerAuthorizer::check_permission(const string_& dsid,
                                        const string_& permission_token,
                                        MessageType method, const Path& path,
                                        CheckPermissionCallback&& callback) {
  _strand->post([ =, callback = std::move(callback) ]() {
    callback(PermissionLevel::CONFIG);
  });
}
}

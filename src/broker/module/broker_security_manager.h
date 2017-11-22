#ifndef DSA_BROKER_SECURITY_MANAGER_H
#define DSA_BROKER_SECURITY_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"
#include "module/security_manager.h"

namespace dsa {

class BrokerSecurityManager : public SecurityManager {
 protected:
  LinkStrandRef _strand;

 public:
  explicit BrokerSecurityManager(LinkStrandRef strand);

  void get_client(const string_& dsid, const string_& auth_token,
                  GetClientCallback&& callback) override;

  void check_permission(const string_& dsid, const string_& permission_token,
                        MessageType method, const Path& path,
                        CheckPermissionCallback&& callback) override;
};
}

#endif  // DSA_BROKER_SESSION_MANAGER_H

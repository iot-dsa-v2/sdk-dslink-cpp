#ifndef DSA_SDK_MODULE_SIMPLE_SECURITY_MANAGER_H
#define DSA_SDK_MODULE_SIMPLE_SECURITY_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/link_strand.h"

#include "module/authorizer.h"
#include "module/client_manager.h"

namespace dsa {

class SimpleAuthorizer : public Authorizer {
 protected:
  LinkStrandRef _strand;

 public:
  explicit SimpleAuthorizer(LinkStrandRef strand);
  void check_permission(const ClientInfo& client_info, const string_& permission_token,
                        MessageType method, const Path& path,
                        CheckPermissionCallback&& callback) override;
};

class SimpleClientManager : public ClientManager {
 public:
  void get_client(const string_& dsid, const string_& auth_token,
                  ClientInfo::GetClientCallback&& callback) override;
};

class AsyncSimpleClientManager : public ClientManager {
 protected:
  LinkStrandRef _strand;

 public:
  explicit AsyncSimpleClientManager(LinkStrandRef strand);

  void get_client(const string_& dsid, const string_& auth_token,
                  ClientInfo::GetClientCallback&& callback) override;

};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SIMPLE_SECURITY_MANAGER_H

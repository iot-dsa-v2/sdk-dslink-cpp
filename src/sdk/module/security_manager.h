#ifndef DSA_SDK_MODULE_SECURITY_MANAGER_H_
#define DSA_SDK_MODULE_SECURITY_MANAGER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>
#include <string>

#include "core/client_info.h"
#include "message/enums.h"
#include "util/enable_intrusive.h"

#include "storage.h"

namespace dsa {

class Path;


class SecurityManager {
 public:
  typedef std::function<void(const ClientInfo client, bool error)>
      GetClientCallback;
  typedef std::function<void(PermissionLevel permission)>
      CheckPermissionCallback;

  virtual void get_client(const std::string& dsid,
                          const std::string& auth_token,
                          GetClientCallback&& callback) = 0;

  virtual void check_permission(const std::string& dsid,
                                const std::string& permission_token,
                                MessageType method, const Path& path,
                                CheckPermissionCallback&& callback) = 0;
  virtual ~SecurityManager(){};
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SECURITY_MANAGER_H_

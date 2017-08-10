#ifndef DSA_SDK_MODULE_SECURITY_MANAGER_H_
#define DSA_SDK_MODULE_SECURITY_MANAGER_H_

#include <functional>
#include <string>

#include "message/enums.h"

namespace dsa {

struct ClientInfo {
  const std::string dsid;
  const std::string permission_token;
  const bool multi_session;

  ClientInfo(const std::string dsid, const std::string permission_token,
             const bool multi_session = false)
      : dsid(dsid),
        permission_token(permission_token),
        multi_session(multi_session) {}
};

class SecurityManager {
 public:
  typedef std::function<void(const ClientInfo* client)> GetClientCallback;
  typedef std::function<void(uint8_t permission)> CheckPermissionCallback;

  virtual void get_client(const std::string& dsid,
                          const std::string& auth_token,
                          const GetClientCallback& callback) = 0;

  virtual void check_permission(const std::string& dsid,
                                const std::string& permission_token,
                                MessageType method, const std::string& path,
                                const CheckPermissionCallback& callback) = 0;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SECURITY_MANAGER_H_

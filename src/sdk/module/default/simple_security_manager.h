#ifndef DSA_SDK_MODULE_DIMPLE_SECURITY_MANAGER_H_
#define DSA_SDK_MODULE_DIMPLE_SECURITY_MANAGER_H_

#include "../security_manager.h"

namespace dsa {

class SimpleSecurityManager : public SecurityManager {
 public:
  typedef std::function<void(const ClientInfo* client)> GetClientCallback;
  typedef std::function<void(uint8_t permission)> CheckPermissionCallback;

  void get_client(const std::string& dsid, const std::string& auth_token,
                  const GetClientCallback& callback) override;

  void check_permission(const std::string& dsid,
                        const std::string& permission_token, MessageType method,
                        const std::string& path,
                        const CheckPermissionCallback& callback) override;

};  // namespace dsa

}  // namespace dsa

#endif  // DSA_SDK_MODULE_DIMPLE_SECURITY_MANAGER_H_

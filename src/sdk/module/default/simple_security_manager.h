#ifndef DSA_SDK_MODULE_DIMPLE_SECURITY_MANAGER_H_
#define DSA_SDK_MODULE_DIMPLE_SECURITY_MANAGER_H_

#include <boost/asio.hpp>

#include "../security_manager.h"

namespace dsa {

class SimpleSecurityManager : public SecurityManager {
 public:
  void get_client(const std::string& dsid, const std::string& auth_token,
                  const GetClientCallback& callback) override;

  void check_permission(const std::string& dsid,
                        const std::string& permission_token, MessageType method,
                        const std::string& path,
                        const CheckPermissionCallback& callback) override;
};

class AsyncSimpleSecurityManager : public SecurityManager {
 protected:
  boost::asio::io_service::strand& _strand;

 public:
  AsyncSimpleSecurityManager(boost::asio::io_service::strand& strand);

  void get_client(const std::string& dsid, const std::string& auth_token,
                  const GetClientCallback& callback) override;

  void check_permission(const std::string& dsid,
                        const std::string& permission_token, MessageType method,
                        const std::string& path,
                        const CheckPermissionCallback& callback) override;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_DIMPLE_SECURITY_MANAGER_H_

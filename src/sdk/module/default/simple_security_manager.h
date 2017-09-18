#ifndef DSA_SDK_MODULE_DIMPLE_SECURITY_MANAGER_H_
#define DSA_SDK_MODULE_DIMPLE_SECURITY_MANAGER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/io_service.hpp>

#include "../security_manager.h"

namespace dsa {

class SimpleSecurityManager : public SecurityManager {
 public:
  void get_client(const std::string& dsid, const std::string& auth_token,
                  GetClientCallback&& callback) override;

  void check_permission(const std::string& dsid,
                        const std::string& permission_token, MessageType method,
                        const Path& path,
                        CheckPermissionCallback&& callback) override;
};

class AsyncSimpleSecurityManager : public SecurityManager {
 protected:
  std::unique_ptr<boost::asio::io_service::strand> _strand;

 public:
  explicit AsyncSimpleSecurityManager(boost::asio::io_service::strand* strand);

  void get_client(const std::string& dsid, const std::string& auth_token,
                  GetClientCallback&& callback) override;

  void check_permission(const std::string& dsid,
                        const std::string& permission_token, MessageType method,
                        const Path& path,
                        CheckPermissionCallback&& callback) override;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_DIMPLE_SECURITY_MANAGER_H_

#ifndef DSA_SDK_MODULE_SECURITY_MANAGER_H_
#define DSA_SDK_MODULE_SECURITY_MANAGER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>
#include <string>

#include "core/client_info.h"
#include "util/enable_ref.h"

#include "storage.h"

namespace dsa {



class ClientManager : public DestroyableRef<ClientManager> {
 public:
  typedef std::function<void(const ClientInfo client, bool error)>
      GetClientCallback;

  virtual void get_client(const string_& dsid, const string_& auth_token,
                          GetClientCallback&& callback) = 0;

  virtual ~ClientManager(){};
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SECURITY_MANAGER_H_

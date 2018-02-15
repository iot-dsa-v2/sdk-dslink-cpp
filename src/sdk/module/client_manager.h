#ifndef DSA_SDK_MODULE_SECURITY_MANAGER_H
#define DSA_SDK_MODULE_SECURITY_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>
#include <string>

#include "util/client_info.h"
#include "util/enable_ref.h"

#include "storage.h"

namespace dsa {



class ClientManager : public DestroyableRef<ClientManager> {
 public:

  virtual void get_client(const string_& dsid, const string_& auth_token,
                          ClientInfo::GetClientCallback&& callback) = 0;

  virtual void set_strand(LinkStrandRef strand){};


  virtual ~ClientManager(){};
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SECURITY_MANAGER_H

#ifndef DSA_SDK_AUTHORIZER_H
#define DSA_SDK_AUTHORIZER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>
#include <string>

#include "util/enums.h"
#include "storage.h"
#include "util/client_info.h"
#include "util/enable_ref.h"

namespace dsa {

class Path;

class Authorizer : public DestroyableRef<Authorizer> {
 public:
  typedef std::function<void(PermissionLevel permission)>
      CheckPermissionCallback;

  virtual void check_permission(const ClientInfo& client_info,
                                const string_& permission_token,
                                MessageType method, const Path& path,
                                CheckPermissionCallback&& callback) = 0;
  virtual ~Authorizer(){};
};
}

#endif  // DSA_SDK_AUTHORIZER_H

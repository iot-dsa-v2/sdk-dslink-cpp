#ifndef DSA_SDK_AUTHORIZER_H
#define DSA_SDK_AUTHORIZER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>
#include <string>

#include "message/enums.h"
#include "util/enable_ref.h"

#include "storage.h"

namespace dsa {

class Path;

class Authorizer : public DestroyableRef<ClientManager> {
 public:
  typedef std::function<void(PermissionLevel permission)>
      CheckPermissionCallback;

  virtual void check_permission(const string_& dsid,
                                const string_& permission_token,
                                MessageType method, const Path& path,
                                CheckPermissionCallback&& callback) = 0;
  virtual ~Authorizer(){};
};
}

#endif  // DSA_SDK_AUTHORIZER_H

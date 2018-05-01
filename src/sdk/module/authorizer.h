#ifndef DSA_SDK_AUTHORIZER_H
#define DSA_SDK_AUTHORIZER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "util/enable_ref.h"
#include "util/enums.h"

namespace dsa {

class Path;

class Authorizer : public DestroyableRef<Authorizer> {
 public:
  typedef std::function<void(PermissionLevel permission)>
      CheckPermissionCallback;

  virtual void check_permission(const string_& id, const string_& role,
                                const string_& permission_token,
                                MessageType method, const Path& path,
                                CheckPermissionCallback&& callback) = 0;
  virtual ~Authorizer(){};
};
}  // namespace dsa

#endif  // DSA_SDK_AUTHORIZER_H

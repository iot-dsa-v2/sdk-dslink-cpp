#ifndef DSA_SDK_MODULE_SESSION_MANAGER_H
#define DSA_SDK_MODULE_SESSION_MANAGER_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/session.h"
#include "util/enable_ref.h"

namespace dsa {

class SessionManager : public DestroyableRef<SessionManager> {
 public:
  virtual ~SessionManager() = default;
  virtual void get_session(const string_ &dsid, const string_ &auth_token,
                           const string_ &session_id,
                           Session::GetSessionCallback &&callback) = 0;
};

}  // namespace dsa

#endif  // DSA_SDK_MODULE_SESSION_MANAGER_H

#ifndef DSA_SDK_USER_SESSION_H
#define DSA_SDK_USER_SESSION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>
#include "dsa_common.h"
#include "util/client_info.h"

namespace dsa {
class ClientInfo;
class UserSession {
 private:
  ClientInfo user;
  string_ dguser_cookie;
  string_ dgsession_cookie;
  string_ dgtoken_cookie;
  uint64_t timeout;
  bool remember_me;

 public:

};
}

#endif  // DSA_SDK_USER_SESSION_H

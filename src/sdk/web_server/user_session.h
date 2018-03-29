#ifndef DSA_SDK_USER_SESSION_H
#define DSA_SDK_USER_SESSION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <unordered_map>
#include "dsa_common.h"
#include "util/client_info.h"
#include "util/date_time.h"

namespace dsa {
class UserSession {
 private:
  string_ _dguser_cookie;
  string_ _dgsession_cookie;
  string_ _dgtoken_cookie;
  int64_t _timeout;
  bool _remember_me = false;

 public:
  explicit UserSession(string_ dgsession_cookie, string_ dgtoken_cookie = "",
                       string_ dguser_cookie = "")
      : _timeout(DateTime::ms_since_epoch()),
        _dgsession_cookie(std::move(dgsession_cookie)),
        _dgtoken_cookie(std::move(dgtoken_cookie)),
        _dguser_cookie(std::move(dguser_cookie)) {}

  void update_session_timeout() { _timeout = DateTime::ms_since_epoch(); }
  int64_t get_timeout() { return _timeout; }

  void set_user_cookie(string_ dguser_cookie) {
    _dguser_cookie = std::move(dguser_cookie);
  }
  void set_session_cookie(string_ dgsession_cookie) {
    _dgsession_cookie = std::move(dgsession_cookie);
  }
  void set_token_cookie(string_ dgtoken_cookie) {
    _dgtoken_cookie = std::move(dgtoken_cookie);
  }
  string_ get_user_cookie() { return _dguser_cookie; }
  string_ get_session_cookie() { return _dgsession_cookie; }
  string_ get_token_cookie() { return _dgtoken_cookie; }
};
}

#endif  // DSA_SDK_USER_SESSION_H

#ifndef PROJECT_CLIENT_INFO_H
#define PROJECT_CLIENT_INFO_H

#if defined(_MSC_VER)
#pragma once
#endif

namespace dsa {

struct ClientInfo {
  typedef std::function<void(const ClientInfo client, bool error)>
    GetClientCallback;

  string_ dsid;
  string_ permission_token;
  string_ responder_path;
  size_t max_session;

  int32_t max_queue_size = 1000000;
  int32_t default_queue_size = 65536;

  int32_t max_queue_time = 3600 * 24;
  int32_t default_queue_time = 60;

  ClientInfo(const string_ dsid = "",
             const string_ permission_token = "", size_t max_session = 1)
      : dsid(dsid),
        permission_token(permission_token),
        max_session(max_session) {}
};
}
#endif  // PROJECT_CLIENT_INFO_H

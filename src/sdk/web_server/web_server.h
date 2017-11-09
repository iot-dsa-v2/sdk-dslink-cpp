#ifndef DSA_SDK_WEB_SERVER_H_
#define DSA_SDK_WEB_SERVER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include "core/server.h"

#include <functional>

namespace dsa {

class WebServer : public Server {
 private:
  size_t _thread_count;

 public:
  typedef std::function<void(WebServer&)> HttpCallback;
  typedef std::function<void(WebServer&)> WsCallback;

  WebServer(WrapperConfig &, size_t thread_count = 4);
  ~WebServer();

  void start() override;
  void destroy_impl() override;
  string_ type() override { return "HTTP"; }

  // HTTP server specific methods
  void add_http_handler(const string_& path, HttpCallback&& callback);
  void add_ws_handler(const string_& path, WsCallback&& callback);

  // util functions
 public:
  void send_error(int error_code, const string_ msg = "");
};
}  // namespace dsa

#endif  // DSA_SDK_WEB_SERVER_H_

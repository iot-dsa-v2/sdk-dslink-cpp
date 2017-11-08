#ifndef DSA_SDK_WEB_SERVER_H_
#define DSA_SDK_WEB_SERVER_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <functional>

namespace dsa {

class WebServer {
 public:
  typedef std::function<void(WebServer&)> HttpCallback;
  typedef std::function<void(WebServer&)> WsCallback;

  void add_http_handler(const string_ & path, HttpCallback&& callback);
  void add_ws_handler(const string_ & path, WsCallback&& callback);


  // util functions
public:
  void send_error(int error_code, const string_ msg = "");
  

};
}

#endif  // DSA_SDK_WEB_SERVER_H_
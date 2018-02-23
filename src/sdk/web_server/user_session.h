#ifndef DSA_SDK_USER_SESSION_H
#define DSA_SDK_USER_SESSION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio.hpp>
#include <boost/beast/http.hpp>
#include <memory>

namespace http = boost::beast::http;

namespace dsa {

class WebServer;

class UserSession : public std::enable_shared_from_this<UserSession> {
 private:
  http::request<http::string_body> _req;
  http::response<http::dynamic_body> response_;
  bool file_flag = false;
  http::response<http::file_body> resp_;
  WebServer &_web_server;
  boost::asio::ip::tcp::socket _socket;
  string_ const doc_root = "/home/yg/Downloads/dsa/dglux-server/www/";

 public:
  explicit UserSession(
      WebServer &web_server, boost::asio::ip::tcp::socket socket,
      boost::beast::http::request<boost::beast::http::string_body> req);
  void process_request();
  void create_response();
  void write_response();
};

class LoginCallback {
 private:
 public:
  // explicit LoginCallback() = default;
  auto operator()(WebServer &web_server, boost::asio::ip::tcp::socket &&socket,
                  http::request<http::string_body> &&req) {
    // UserSession us{web_server, std::move(socket), std::move(req)};
    // us.process_request();
  }
};
}

#endif  // DSA_SDK_USER_SESSION_H

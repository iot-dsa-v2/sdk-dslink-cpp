#ifndef DSA_SDK_HTTP_REQUEST_H
#define DSA_SDK_HTTP_REQUEST_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "dsa_common.h"
#include "fields_alloc.h"
#include "http_response.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <list>
#include <memory>
#include <string>

namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

namespace dsa {

class WebServer;

class HttpRequest {
 private:
  using alloc_t = fields_alloc<char>;
  using request_body_t =
      http::basic_dynamic_body<boost::beast::flat_static_buffer<1024 * 1024>>;

  std::string _doc_root;

  tcp::socket _socket;

  alloc_t _alloc = alloc_t{8192};

  http::request<request_body_t, http::basic_fields<alloc_t>> _req;

  WebServer& _web_server;

  void send_bad_response(http::status status, std::string const& error);
  void send_file(boost::beast::string_view target);

 public:
  std::shared_ptr<HttpStringResponse> str_response = nullptr;
  std::shared_ptr<HttpFileResponse> file_response = nullptr;

  explicit HttpRequest(
      WebServer& web_server, boost::asio::ip::tcp::socket socket,
      http::request<request_body_t, http::basic_fields<alloc_t>> _req);

  template <typename T>
  void create_response() {
    if (std::is_same<T, HttpStringResponse>::value) {
      if (!str_response) str_response = std::make_shared<HttpStringResponse>();
    } else if (std::is_same<T, HttpFileResponse>::value) {
      if (!file_response) file_response = std::make_shared<HttpFileResponse>();
    }
  }

  template <typename T,
            typename = typename std::enable_if<
                std::is_same<T, HttpStringResponse>::value, bool>::type>
  std::shared_ptr<HttpStringResponse> get_response() {
    create_response<T>();
    return str_response;
  }

  template <typename T,
            typename = typename std::enable_if<
                std::is_same<T, HttpFileResponse>::value, bool>::type>
  std::shared_ptr<HttpFileResponse> get_response() {
    create_response<T>();
    return file_response;
  }

  void redirect_handler(const string_& location, const string_& message);
  void not_found_handler(const string_& error);
  void rewrite_handler(const string_& redirect_path);
  void file_server_handler(const string_& target);
  void authentication_handler();
  void timeout_handler();

  bool is_authenticated(const string_& username, const string_& password);
  bool is_authenticated();
  bool is_session_active();
  void create_session();
};
}

#endif  // DSA_SDK_HTTP_REQUEST_H

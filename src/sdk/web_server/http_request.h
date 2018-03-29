#ifndef DSA_SDK_HTTP_REQUEST_H
#define DSA_SDK_HTTP_REQUEST_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "dsa_common.h"
#include "fields_alloc.h"
#include "http_connection.h"
#include "http_response.h"
#include "ssl_stream.h"

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
 protected:
  using alloc_t = fields_alloc<char>;
  using request_body_t =
      http::basic_dynamic_body<boost::beast::flat_static_buffer<1024 * 1024>>;

  std::string _doc_root;

  bool _is_secured = false;

  alloc_t _alloc = alloc_t{8192};

  http::request<request_body_t, http::basic_fields<alloc_t>> _req;

  WebServer& _web_server;

  void send_bad_response(http::status status, std::string const& error);
  void send_file(boost::beast::string_view target);

 public:
  std::shared_ptr<HttpStringResponse> str_response = nullptr;
  std::shared_ptr<HttpFileResponse> file_response = nullptr;

  HttpRequest(WebServer& web_server,
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
  void login_handler();
  void authentication_handler();
  void timeout_handler();

  virtual void writer(shared_ptr_<HttpStringResponse>) = 0;
  virtual void writer(shared_ptr_<HttpFileResponse>) = 0;

  string_ get_session_cookie();
  string_ get_token_cookie();
  string_ get_user_cookie();

  bool is_authenticated();
  bool is_authenticated(const string_& username, const string_& password);
  bool is_session_active();
  bool is_session_active(const string_& session_cookie);
  string_ create_new_session();

  void destroy() { _alloc.pool_.destroy(); }

  virtual ~HttpRequest() { destroy(); }
};

class SecureHttpRequest
    : public HttpRequest,
      public std::enable_shared_from_this<SecureHttpRequest> {
  ssl_stream<tcp::socket> _stream;

 public:
  SecureHttpRequest(
      WebServer& web_server, ssl_stream<tcp::socket>&& socket,
      http::request<request_body_t, http::basic_fields<alloc_t>> _req);

  ssl_stream<tcp::socket>& stream() { return _stream; }

  ssl_stream<tcp::socket> release_stream() { return std::move(_stream); }

  void writer(shared_ptr_<HttpStringResponse> resp) override;
  void writer(shared_ptr_<HttpFileResponse> resp) override;

  ~SecureHttpRequest() override = default;
};

class PlainHttpRequest : public HttpRequest,
                         public std::enable_shared_from_this<PlainHttpRequest> {
  tcp::socket _socket;

 public:
  PlainHttpRequest(
      WebServer& web_server, tcp::socket&& socket,
      http::request<request_body_t, http::basic_fields<alloc_t>> _req);

  tcp::socket& stream() { return _socket; }

  tcp::socket release_stream() { return std::move(_socket); }

  void writer(shared_ptr_<HttpStringResponse> resp) override;
  void writer(shared_ptr_<HttpFileResponse> resp) override;

  ~PlainHttpRequest() override = default;
};
}

#endif  // DSA_SDK_HTTP_REQUEST_H

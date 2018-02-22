#ifndef DSA_SDK_HTTP_REQUEST_H
#define DSA_SDK_HTTP_REQUEST_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "dsa_common.h"
#include "fields_alloc.h"
#include "web_server.h"

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

class HttpRequest {
 private:
  using alloc_t = fields_alloc<char>;
  using request_body_t =
      http::basic_dynamic_body<boost::beast::flat_static_buffer<1024 * 1024>>;

  std::string _doc_root;

  tcp::socket _socket;

  alloc_t _alloc = alloc_t{8192};

  http::request<request_body_t, http::basic_fields<alloc_t>> _req;

  boost::optional<
      http::response<http::string_body, http::basic_fields<alloc_t>>>
      _str_resp;

  boost::optional<
      http::response_serializer<http::string_body, http::basic_fields<alloc_t>>>
      _str_serializer;

  boost::optional<http::response<http::file_body, http::basic_fields<alloc_t>>>
      _file_resp;

  boost::optional<
      http::response_serializer<http::file_body, http::basic_fields<alloc_t>>>
      _file_serializer;

  WebServer& _web_server;

  void send_bad_response(http::status status, std::string const& error);
  void send_file(boost::beast::string_view target);
  void string_writer();
  void file_writer();

 public:
  explicit HttpRequest(
      WebServer& web_server, boost::asio::ip::tcp::socket socket,
      http::request<request_body_t, http::basic_fields<alloc_t>> _req);

  void redirect_handler(const string_& location, const string_& message);
  void not_found_handler(const string_& error);
  void rewrite_handler(const string_& redirect_path);
  void file_server_handler(const string_& target);
  void authentication_handler();
  void timeout_handler();
};
}

#endif  // DSA_SDK_HTTP_REQUEST_H

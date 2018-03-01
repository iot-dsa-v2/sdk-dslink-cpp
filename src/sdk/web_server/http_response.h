#ifndef DSA_SDK_HTTP_RESPONSE_H
#define DSA_SDK_HTTP_RESPONSE_H

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

class HttpResponse {
 private:
  using alloc_t = fields_alloc<char>;
  using request_body_t =
      http::basic_dynamic_body<boost::beast::flat_static_buffer<1024 * 1024>>;

  alloc_t _alloc = alloc_t{8192};

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

 public:
  explicit HttpResponse();

  boost::optional<http::response<http::file_body, http::basic_fields<alloc_t>>>&
  get_file_response();
  void prepare_file_response();
  void prepare_file_serializer();
  void file_writer(tcp::socket&& _socket);

  boost::optional<
      http::response<http::string_body, http::basic_fields<alloc_t>>>&
  get_string_response();
  void prepare_string_response();
  void prepare_string_serializer();
  void string_writer(tcp::socket&& _socket);
};
}

#endif  // DSA_SDK_HTTP_RESPONSE_H

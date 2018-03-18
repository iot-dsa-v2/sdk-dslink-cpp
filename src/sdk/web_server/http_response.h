#ifndef DSA_SDK_HTTP_RESPONSE_H
#define DSA_SDK_HTTP_RESPONSE_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "dsa_common.h"
#include "fields_alloc.h"
#include "ssl_stream.h"
#include "web_server.h"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/optional.hpp>
#include <memory>

namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

namespace dsa {

template <typename InternalType>
struct SelfType {
  InternalType& internal_resp_type() {
    return static_cast<InternalType&>(*this);
  }
  InternalType const& internal_resp_type() const {
    return static_cast<InternalType const&>(*this);
  }
};

template <typename ResponseType>
class HttpResponse : public SelfType<ResponseType> {
 protected:
  using alloc_t = fields_alloc<char>;
  alloc_t _alloc = alloc_t{8192};

 public:
  decltype(auto) prep_response() {
    return this->internal_resp_type()._prepare_response();
  }

  void init_response() { this->internal_resp_type()._init_response(); }

  void prep_serializer() { this->internal_resp_type()._prepare_serializer(); }

  void destroy() { _alloc.pool_.destroy(); }

  virtual ~HttpResponse() { destroy(); }
};

class HttpStringResponse
    : public HttpResponse<HttpStringResponse>,
      public std::enable_shared_from_this<HttpStringResponse> {
 public:
  //  alloc_t _alloc = alloc_t{8192};

  boost::optional<
      http::response_serializer<http::string_body, http::basic_fields<alloc_t>>>
      _str_serializer;

  boost::optional<
      http::response<http::string_body, http::basic_fields<alloc_t>>>
      _str_resp;

  std::mutex _mutex;

 public:
  boost::optional<
      http::response<http::string_body, http::basic_fields<alloc_t>>>&
  _prepare_response() {
    return _str_resp;
  };

  void _init_response() {
    _str_resp.emplace(std::piecewise_construct, std::make_tuple(),
                      std::make_tuple(_alloc));
  }

  void _prepare_serializer() { _str_serializer.emplace(*_str_resp); }

  ~HttpStringResponse() override = default;
};

class HttpFileResponse : public HttpResponse<HttpFileResponse>,
                         public std::enable_shared_from_this<HttpFileResponse> {
 public:
  //  alloc_t _alloc = alloc_t{8192};

  boost::optional<
      http::response_serializer<http::file_body, http::basic_fields<alloc_t>>>
      _file_serializer;

  boost::optional<http::response<http::file_body, http::basic_fields<alloc_t>>>
      _file_resp;

  std::mutex _mutex;

 public:
  boost::optional<http::response<http::file_body, http::basic_fields<alloc_t>>>&
  _prepare_response() {
    return _file_resp;
  };

  void _init_response() {
    _file_resp.emplace(std::piecewise_construct, std::make_tuple(),
                       std::make_tuple(_alloc));
  }

  void _prepare_serializer() { _file_serializer.emplace(*_file_resp); }

  ~HttpFileResponse() override = default;
};
}

#endif  // DSA_SDK_HTTP_RESPONSE_H

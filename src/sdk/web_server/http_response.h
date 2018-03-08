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
#include <boost/optional.hpp>
#include <memory>

namespace ip = boost::asio::ip;
using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;

namespace dsa {

template <typename InternalType>
struct Self {
  InternalType& internal_resp_type() {
    return static_cast<InternalType&>(*this);
  }
  InternalType const& internal_resp_type() const {
    return static_cast<InternalType const&>(*this);
  }
};

template <typename ResponseType>
class HttpResponse : public Self<ResponseType> {
 protected:
  using alloc_t = fields_alloc<char>;

 public:
  decltype(auto) prep_response() {
    return this->internal_resp_type()._prepare_response();
  }

  void init_response() { this->internal_resp_type()._init_response(); }

  void prep_serializer() { this->internal_resp_type()._prepare_serializer(); }

  void writer(tcp::socket&& _socket) {
    this->internal_resp_type()._writer(std::move(_socket));
  }
};

class HttpStringResponse : public HttpResponse<HttpStringResponse> {
 private:
  alloc_t _alloc = alloc_t{8192};

  boost::optional<
      http::response_serializer<http::string_body, http::basic_fields<alloc_t>>>
      _str_serializer;

  boost::optional<
      http::response<http::string_body, http::basic_fields<alloc_t>>>
      _str_resp;

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

  void _writer(tcp::socket&& _socket) {
    http::async_write(
        _socket, *_str_serializer,
        [this, &_socket](boost::beast::error_code ec, std::size_t) {
          _socket.shutdown(tcp::socket::shutdown_send, ec);
          _str_serializer.reset();
          _str_resp.reset();
        });
  }
};

class HttpFileResponse : public HttpResponse<HttpFileResponse> {
 private:
  alloc_t _alloc = alloc_t{8192};

  boost::optional<
      http::response_serializer<http::file_body, http::basic_fields<alloc_t>>>
      _file_serializer;

  boost::optional<http::response<http::file_body, http::basic_fields<alloc_t>>>
      _file_resp;

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

  void _writer(tcp::socket&& _socket) {
    http::async_write(
        _socket, *_file_serializer,
        [this, &_socket](boost::beast::error_code ec, std::size_t) {
          _socket.shutdown(tcp::socket::shutdown_send, ec);
          _file_serializer.reset();
          _file_resp.reset();
        });
  }
};
}

#endif  // DSA_SDK_HTTP_RESPONSE_H

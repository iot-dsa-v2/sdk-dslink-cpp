#ifndef DSA_SDK_NETWORK_HTTP_CONNECTION_H_
#define DSA_SDK_NETWORK_HTTP_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

#include "core/editable_strand.h"
#include "module/session_manager.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security_manager.h"

#include "util/enable_shared.h"

using tcp = boost::asio::ip::tcp;
namespace http = boost::beast::http;  // from <boost/beast/http.hpp>

namespace dsa {

// Web server side connection.
// Handles server side of DSA handshake and starts read loop.
class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
 private:
  tcp::socket _socket;
  boost::beast::flat_buffer _buffer;
  http::request<http::string_body> _req;
  boost::asio::io_service& _io_service;

 public:
  HttpConnection(boost::asio::io_service& io_service)
      : _io_service(io_service), _socket(io_service) {}
  void accept();
  tcp::socket& socket() { return _socket; }
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_HTTP_CONNECTION_H_

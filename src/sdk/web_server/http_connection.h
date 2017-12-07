#ifndef DSA_SDK_NETWORK_HTTP_CONNECTION_H_
#define DSA_SDK_NETWORK_HTTP_CONNECTION_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

//#include "core/editable_strand.h"
//#include "module/default/console_logger.h"
//#include "module/default/simple_security_manager.h"
//#include "module/session_manager.h"

#include "util/enable_shared.h"

namespace dsa {

class WebServer;

// Web server side connection.
class HttpConnection : public std::enable_shared_from_this<HttpConnection> {
 private:
  WebServer& _web_server;
  boost::asio::ip::tcp::socket _socket;
  boost::beast::flat_buffer _buffer;
  boost::beast::http::request<boost::beast::http::string_body> _req;
  boost::asio::io_service& _io_service;

 public:
  HttpConnection(WebServer& web_server, boost::asio::io_service& io_service)
      : _web_server(web_server), _io_service(io_service), _socket(io_service) {}
  void accept();
  boost::asio::ip::tcp::socket& socket() { return _socket; }
};
}  // namespace dsa

#endif  // DSA_SDK_NETWORK_HTTP_CONNECTION_H_

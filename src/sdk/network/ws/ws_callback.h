#ifndef DSA_SDK_WS_CALLBACK_H
#define DSA_SDK_WS_CALLBACK_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/beast/http.hpp>

#include "core/editable_strand.h"
#include "network/ws/ws_server_connection.h"
#include "web_server/websocket.h"

#include <mutex>

namespace dsa {

class DsaWsCallback {
 private:
  SharedLinkStrandRef _shared_strand;
  std::mutex _mutex;

 public:
  DsaWsCallback(const LinkStrandRef& link_strand)
      : _shared_strand(share_strand_(link_strand)) {}

  auto operator()(
      boost::asio::io_context& io_context,
      std::unique_ptr<Websocket>&& websocket,
      http::request<boost::beast::http::string_body>&& req) {
    shared_ptr_<Connection> connection;

    std::lock_guard<std::mutex> lock(_mutex);
    connection =
        make_shared_<WsServerConnection>(std::move(websocket), _shared_strand);
    std::dynamic_pointer_cast<WsConnection>(connection)->accept();

    // return connection;
    return std::move(connection);
  }
};
}  // namespace dsa

#endif  // DSA_SDK_WS_CALLBACK_H

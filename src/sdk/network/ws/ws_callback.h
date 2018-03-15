#ifndef DSA_SDK_WS_CALLBACK_H
#define DSA_SDK_WS_CALLBACK_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/beast/http.hpp>

#include "core/editable_strand.h"
#include "network/ws/ws_server_connection.h"
#include "web_server/websocket.h"
#include "web_server/http_request.h"

#include <mutex>

namespace dsa {

class DsaWsCallback {
 private:
  LinkStrandRef& _link_strand;
  std::mutex _mutex;

 public:
  DsaWsCallback(LinkStrandRef& link_strand) : _link_strand(link_strand) {}

  auto operator()(
      boost::asio::io_context& io_context,
      std::unique_ptr<Websocket>&& websocket,
      http::request<request_body_t, http::basic_fields<alloc_t>>&& req) {
    shared_ptr_<Connection> connection;

    //    std::lock_guard<std::mutex> lock(_mutex);
    connection =
        make_shared_<WsServerConnection>(std::move(websocket), _link_strand);
    std::dynamic_pointer_cast<WsConnection>(connection)->accept();

    return std::move(connection);
  }
};
}  // namespace dsa

#endif  // DSA_SDK_WS_CALLBACK_H

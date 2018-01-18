#ifndef DSA_SDK_WS_CALLBACK_H_
#define DSA_SDK_WS_CALLBACK_H_

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/strand.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

#include "core/editable_strand.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security_manager.h"
#include "module/default/simple_session_manager.h"
#include "module/session_manager.h"
#include "network/ws/ws_server_connection.h"

namespace websocket =
    boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace dsa {

class DsaWsCallback {
 private:
  LinkStrandRef& _link_strand;

 public:
  DsaWsCallback(LinkStrandRef& link_strand) : _link_strand(link_strand) {}

  auto operator()(
      boost::asio::io_context& io_context,
      boost::asio::ip::tcp::socket&& socket,
      boost::beast::http::request<boost::beast::http::string_body>&& req) {
    auto connection = make_shared_<WsServerConnection>(
        *new websocket_stream{std::move(socket)}, _link_strand);

    connection->socket().async_accept(
        req, [conn = connection, this](const boost::system::error_code& error) {

          // TODO: run within the strand?
          conn->accept();

          return;
        });

    return connection;
  }
};
}  // namespace dsa

#endif  // DSA_SDK_WS_CALLBACK_H_

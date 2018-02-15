#ifndef DSA_SDK_WS_CALLBACK_H
#define DSA_SDK_WS_CALLBACK_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/asio/strand.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/websocket.hpp>

#include "core/editable_strand.h"
#include "module/default/console_logger.h"
#include "module/default/simple_security.h"
#include "module/default/simple_session_manager.h"
#include "module/session_manager.h"
#include "network/ws/ws_server_connection.h"
#include "network/ws/wss_server_connection.h"
#include "web_server/websocket.h"

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
      Websocket websocket,
      boost::beast::http::request<boost::beast::http::string_body>&& req) {

    // WSS_TBD
    shared_ptr_<Connection> connection;
    if (websocket.is_secured_stream()) {
      connection = make_shared_<WssServerConnection>(
          *make_shared_<websocket_stream>(websocket.secure_stream()), _link_strand);
    } else {
      connection = make_shared_<WsServerConnection>(
          *make_shared_<websocket_stream>(std::move(websocket.socket())), _link_strand);
    }

    connection->socket().async_accept(
        req,
        [ conn = connection, this ](const boost::system::error_code& error) {

          // TODO: run within the strand?
          conn->accept();

          return;
        });

    return connection;
  }
};
}  // namespace dsa

#endif  // DSA_SDK_WS_CALLBACK_H

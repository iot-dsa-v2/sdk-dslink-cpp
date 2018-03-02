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
#include "web_server/websocket.h"

#include <memory>
#include <mutex>

namespace websocket =
    boost::beast::websocket;  // from <boost/beast/websocket.hpp>

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
      boost::beast::http::request<boost::beast::http::string_body>&& req) {
    shared_ptr_<Connection> connection;

    std::lock_guard<std::mutex> lock(_mutex);
    if (websocket->is_secure_stream()) {
      connection =
          make_shared_<WsServerConnection>(std::move(websocket), _link_strand);
      std::dynamic_pointer_cast<WsConnection>(connection)
          ->ws_stream()
          .secure_stream()
          .async_accept(req, [ conn = connection,
                               this ](const boost::system::error_code& error) {

            // TODO: run within the strand?
            std::dynamic_pointer_cast<WsConnection>(conn)->accept();
            return;
          });
    } else {
      connection =
          make_shared_<WsServerConnection>(std::move(websocket), _link_strand);
      std::dynamic_pointer_cast<WsConnection>(connection)
          ->ws_stream()
          .stream()
          .async_accept(req, [ conn = connection,
                               this ](const boost::system::error_code& error) {

            // TODO: run within the strand?
            std::dynamic_pointer_cast<WsConnection>(conn)->accept();

            return;
          });
    }
    return connection;
  }
};
}  // namespace dsa

#endif  // DSA_SDK_WS_CALLBACK_H

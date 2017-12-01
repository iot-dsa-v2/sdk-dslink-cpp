#include "dsa_common.h"

#include "http_connection.h"

#include <boost/asio/strand.hpp>

#include "network/ws/ws_server_connection.h"
#include "module/default/simple_session_manager.h"

namespace websocket =
    boost::beast::websocket;  // from <boost/beast/websocket.hpp>

namespace dsa {

void HttpConnection::accept() {
  // Read a request
  http::async_read(
      _socket, _buffer, _req,
      // TODO: run within the strand?
      [ this, sthis = shared_from_this() ](
          const boost::system::error_code &error, size_t bytes_transferred) {

        // TODO: check error/termination conditions

        if (websocket::is_upgrade(_req)) {
          // accept the websocket handshake

          auto *config = new EditableStrand(
              new boost::asio::io_service::strand(_io_service),
              make_unique_<ECDH>());
          config->set_session_manager(make_ref_<SimpleSessionManager>(config));
          config->set_security_manager(make_ref_<SimpleSecurityManager>());
          config->set_logger(make_unique_<ConsoleLogger>());
          config->logger().level = Logger::WARN__;

          LinkStrandRef _link_strand;

          _link_strand.reset(config);

          auto conn = make_shared_<WsServerConnection>(
              *new websocket_stream{std::move(_socket)}, _link_strand);

          conn->websocket().async_accept(
              _req, [conn, this](const boost::system::error_code &error) {

                // TODO: run within the strand?
                conn->accept();

                return;
              });  // async_accept
        }
        return;
      });  // async_read
}

}  // namespace dsa

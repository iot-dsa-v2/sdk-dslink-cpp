#include "dsa_common.h"

#include <boost/asio/strand.hpp>
#include <boost/beast/websocket.hpp>

#include "websocket_connection.h"
#include "web_server.h"
#include "network/connection.h"


namespace websocket = boost::beast::websocket;
namespace http = boost::beast::http;

namespace dsa {

    void
    fail(boost::system::error_code ec, char const* what)
    {
        std::cerr << what << ": " << ec.message() << "\n";
    }

    WebsocketConnection::WebsocketConnection(WebServer& web_server, boost::asio::ip::tcp::socket socket)
            : _web_server(web_server), _ws(std::move(socket)), _timer(_ws.get_executor().context(), std::chrono::steady_clock::time_point::max()) {}

    void
    WebsocketConnection::on_accept(boost::system::error_code ec)
    {
        if(ec == boost::asio::error::operation_aborted)
            return;

        if(ec)
            return fail(ec, "accept");

        do_read();
    }

    void
    WebsocketConnection::on_timer(boost::system::error_code ec)
    {
        if(ec && ec != boost::asio::error::operation_aborted)
            return fail(ec, "timer");

        if(_timer.expiry() <= std::chrono::steady_clock::now())
        {
            if(_ws.is_open() && ping_state_ == 0)
            {
                ping_state_ = 1;

                _timer.expires_after(std::chrono::seconds(15));

                _ws.async_ping({}, std::bind(
                                   &WebsocketConnection::on_ping,
                                   shared_from_this(),
                                   std::placeholders::_1));
            }
            else
            {
                _ws.next_layer().shutdown(tcp::socket::shutdown_both, ec);
                _ws.next_layer().close(ec);
                return;
            }
        }

        _timer.async_wait(
                        std::bind(
                                &WebsocketConnection::on_timer,
                                shared_from_this(),
                                std::placeholders::_1));
    }

    void
    WebsocketConnection::activity()
    {
        ping_state_ = 0;

        _timer.expires_after(std::chrono::seconds(15));
    }

    void
    WebsocketConnection::on_ping(boost::system::error_code ec)
    {
        if(ec == boost::asio::error::operation_aborted)
            return;

        if(ec)
            return fail(ec, "ping");

        if(ping_state_ == 1)
        {
            ping_state_ = 2;
        }
        else
        {
            BOOST_ASSERT(ping_state_ == 0);
        }
    }

    void
    WebsocketConnection::do_read()
    {
        _ws.async_read(
                _buffer,
                        std::bind(
                                &WebsocketConnection::on_read,
                                shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2));
    }

    void
    WebsocketConnection::on_read(
            boost::system::error_code ec,
            std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec == boost::asio::error::operation_aborted)
            return;

        if(ec == websocket::error::closed)
            return;

        if(ec)
            fail(ec, "read");

        activity();

        // Echo the message
        _ws.text(_ws.got_text());
        _ws.async_write(
                _buffer.data(),
                        std::bind(
                                &WebsocketConnection::on_write,
                                shared_from_this(),
                                std::placeholders::_1,
                                std::placeholders::_2));
    }

    void
    WebsocketConnection::on_write(
            boost::system::error_code ec,
            std::size_t bytes_transferred)
    {
        boost::ignore_unused(bytes_transferred);

        if(ec == boost::asio::error::operation_aborted)
            return;

        if(ec)
            return fail(ec, "write");

        _buffer.consume(_buffer.size());

        do_read();
    }

    void WebsocketConnection::destroy() {
/*
  if(_connection != nullptr) {
    _connection->destroy();
  }
*/
    }
}  // namespace dsa

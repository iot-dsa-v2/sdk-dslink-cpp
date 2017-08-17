#include "dsa_common.h"

#include "tcp_connection.h"

#include <boost/bind.hpp>

#include "tcp_client.h"
#include "tcp_server.h"
#include "core/session_manager.h"

#define DEBUG 0

namespace dsa {

TcpConnection::TcpConnection(boost::asio::io_service::strand &strand,
                             uint32_t handshake_timeout_ms,
                             const std::string &dsid_prefix,
                             const intrusive_ptr_<ECDH> &ecdh)
    : Connection(strand, handshake_timeout_ms, dsid_prefix, ecdh), 
      _socket(strand.get_io_service()) {}

TcpConnection::TcpConnection(const Config &config)
  : Connection(config), _socket(config.strand.get_io_service()) {}

TcpConnection::TcpConnection(const TcpServer &server)
    : Connection(static_cast<const Server &>(server)),
      _socket(server.get_strand().get_io_service()) {}

TcpConnection::TcpConnection(const TcpClient &client)
    : Connection(static_cast<const Client &>(client)),
      _socket(client.get_strand().get_io_service()) {}

void TcpConnection::close() {
  if (_socket_open.exchange(false)) {
    _socket.close();
  }
  Connection::close();
}

void TcpConnection::read_loop(size_t from_prev,
                              const boost::system::error_code &error,
                              size_t bytes_transferred) {
  // reset deadline timer for each new message
  reset_standard_deadline_timer();

  if (!error) {
    std::cout << std::endl << "in read loop" << std::endl;

    BufferPtr buf = std::move(_read_buffer);
    _write_buffer.reset(new ByteBuffer());

    size_t total_bytes = from_prev + bytes_transferred;
    uint8_t *data = buf->data();
    size_t cur = 0;

    while (cur < total_bytes) {
      // always want full static header instead of just message size to make
      // sure it's a valid message
      if (total_bytes - cur < sizeof(uint32_t)) {
        size_t partial_size = total_bytes - cur;
        _write_buffer->assign(&data[cur], &data[cur] + partial_size);
        _socket.async_read_some(
            boost::asio::buffer(_write_buffer->data() + partial_size,
                                _write_buffer->capacity() - partial_size),
            boost::bind(&TcpConnection::read_loop, share_this<TcpConnection>(),
                        partial_size, boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        return;
      }
      uint32_t message_size = *((uint32_t*)cur);
      if (message_size < total_bytes - cur) {
        size_t partial_size = total_bytes - cur;

        // make sure buffer capacity is enough to read full message
        _write_buffer->resize(message_size);
        _write_buffer->assign(&data[cur], &data[cur] + partial_size);

        // read the rest of the message
        boost::asio::async_read(
            _socket,
            boost::asio::buffer(_write_buffer->data() + partial_size,
                                _write_buffer->capacity() - partial_size),
            boost::bind(&TcpConnection::read_loop, share_this<TcpConnection>(),
                        partial_size, boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred));
        return;
      }

      // post job with message buffer
      Message * message = Message::parse_message(&buf->data()[cur], message_size);

      _strand.post(
          //boost::bind<void>(_message_handler, _session,
          //                  buf->get_shared_buffer(cur, header.message_size))
          [sthis = share_this<TcpConnection>(), message](){
            sthis->post_message(message);
          }
      );

      cur += message_size;
    }

    _socket.async_read_some(
        boost::asio::buffer(_write_buffer->data(), _write_buffer->capacity()),
        boost::bind(&TcpConnection::read_loop, share_this<TcpConnection>(), 0,
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred));
  } else {
    close();
  }
}

void TcpConnection::write_handler(WriteHandler callback,
                                  const boost::system::error_code &error) {
  if (!error)
    callback();
  else {
    if (_session != nullptr)
      _session->close();
    else
      close();
  }
}

void TcpConnection::write(BufferPtr buf, size_t size, WriteHandler callback) {
  // check to see if current number of pending messages is above allowed limit
  if (++_pending_messages > MAX_PENDING_MESSAGE) {
    close();
    return;
  }

  boost::asio::async_write(
      _socket, boost::asio::buffer(buf->data(), size),
      boost::bind(&TcpConnection::write_handler, share_this<TcpConnection>(),
                  callback, boost::asio::placeholders::error));
}

void TcpConnection::start() throw() {
  if (_session == nullptr) {
    close();
    throw std::runtime_error("Error: connection started with no session");
  }

  name();

  _socket.async_read_some(
      boost::asio::buffer(_write_buffer->data(), _write_buffer->capacity()),
      boost::bind(&TcpConnection::read_loop, share_this<TcpConnection>(), 0,
                  boost::asio::placeholders::error, 0));
}

tcp_socket &TcpConnection::socket() { return _socket; }

}  // namespace dsa
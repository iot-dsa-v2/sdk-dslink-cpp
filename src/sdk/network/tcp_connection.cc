#include "tcp_connection.h"
#include <boost/bind.hpp>

#include <boost/bind.hpp>
#include <message/static_header.h>

namespace dsa {


TcpConnection::TcpConnection(boost::asio::io_service &io_service, const SecurityContext &security_context)
    : Connection(io_service), _security_context(security_context), _socket(io_service) {}

void TcpConnection::close() { _socket.close(); }
void TcpConnection::destroy() {
  close();
  EnableShared<Connection>::destory();
}

void TcpConnection::start() {
  _socket.async_read_some(boost::asio::buffer(_buffer->data(), _buffer->capacity()),
                          boost::bind(&TcpConnection::read_loop, this, 0,
                                      boost::asio::placeholders::error,
                                      boost::asio::placeholders::bytes_transferred));
}

void TcpConnection::read_loop(size_t from_prev, const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error) {
    BufferPtr buf = std::move(_buffer);
    _buffer.reset(new Buffer());

    size_t total_bytes = from_prev + bytes_transferred;
    uint8_t *data = buf->data();
    size_t cur = 0;

    while (cur < total_bytes) {
      // always want full static header instead of just message size to make sure it's a valid message
      if (total_bytes - cur < static_header_size) {
        size_t partial_size = total_bytes - cur;
        _buffer->assign(&data[cur], partial_size);
        _socket.async_read_some(boost::asio::buffer(_buffer->data() + partial_size, _buffer->capacity() - partial_size),
                                boost::bind(&TcpConnection::read_loop, this, partial_size,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
        return;
      }

      StaticHeader header(&data[cur]);
      if (header.message_size() < total_bytes - cur) {
        size_t partial_size = total_bytes - cur;

        // make sure buffer capacity is enough to read full message
        _buffer->resize(header.message_size());
        _buffer->assign(&data[cur], partial_size);

        // read the rest of the message
        boost::asio::async_read(_socket,
                                boost::asio::buffer(_buffer->data() + partial_size, _buffer->capacity() - partial_size),
                                boost::bind(&TcpConnection::read_loop, this, partial_size,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred));
        return;
      }

      // post job with message buffer
      io_service.post(boost::bind(&TcpConnection::handle_read, this,
                                  buf->get_message_buffer(cur, header.message_size())));

      cur += header.message_size();
    }

    _socket.async_read_some(boost::asio::buffer(_buffer->data(), _buffer->capacity()),
                            boost::bind(&TcpConnection::read_loop, this, 0,
                                        boost::asio::placeholders::error,
                                        boost::asio::placeholders::bytes_transferred));
  }
}

void TcpConnection::error_check_wrap(WriteCallback callback, const boost::system::error_code &error) {
  if (!error) callback();
}

void TcpConnection::write(BufferPtr buf, size_t size, WriteCallback callback) {
  boost::asio::async_write(_socket, boost::asio::buffer(buf->data(), size),
                           boost::bind<void>(&TcpConnection::error_check_wrap, this, callback,
                                             boost::asio::placeholders::error));
}

tcp_socket &TcpConnection::socket() { return _socket; }

TcpServerConnection::TcpServerConnection(boost::asio::io_service &io_service, const SecurityContext &security_context)
    : TcpConnection(io_service, security_context) {}

void TcpServerConnection::start() {
  socket().async_read_some(boost::asio::buffer(_buffer->data(), _buffer->capacity()),
                           boost::bind(&TcpServerConnection::f0_received, this,
                                       boost::asio::placeholders::error,
                                       boost::asio::placeholders::bytes_transferred));
}

void TcpServerConnection::f0_received(const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error && parse_f0(bytes_transferred)) {

  }
}

void TcpServerConnection::f1_sent(const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error && parse_f0(bytes_transferred)) {

  }
}

void TcpServerConnection::f2_received(const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error && parse_f0(bytes_transferred)) {

  }
}

void TcpServerConnection::f3_sent(const boost::system::error_code &error, size_t bytes_transferred) {
  if (!error && parse_f0(bytes_transferred)) {

  }
}

}  // namespace dsa
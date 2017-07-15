#include <boost/bind.hpp>
#include "tcp_connection.h"

namespace dsa {

TcpConnection::TcpConnection(boost::asio::io_service &io_service, const SecurityContext &security_context)
    : _security_context(security_context), _socket(io_service) {}

void TcpConnection::close() {
  _socket.close();
}

void TcpConnection::error_check_wrap(WriteCallback callback, const boost::system::error_code &error) {
  if (!error) callback();
}

void TcpConnection::write(BufferPtr buf, size_t size, WriteCallback callback) {
//  boost::asio::async_write(_socket, boost::asio::buffer(buf->data(), size),
//                           boost::bind(&TcpConnection::error_check_wrap, this, callback,
//                                       boost::asio::placeholders::error,
//                                       boost::asio::placeholders::bytes_transferred));
}

tcp_socket &TcpConnection::socket() {
  return _socket;
}

TcpServerConnection::TcpServerConnection(boost::asio::io_service &io_service, const SecurityContext &security_context)
  : TcpConnection(io_service, security_context) {}

void TcpServerConnection::start() {
  auto buf = std::make_shared<Buffer>();
//  socket().async_read_some(boost::asio::buffer(buf->data(), buf->capacity()),
//                           boost::bind(&TcpServerConnection::f0_received, this, std::move(buf),
//                                       boost::asio::placeholders::error,
//                                       boost::asio::placeholders::bytes_transferred));
}
}  // namespace dsa
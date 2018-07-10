#include "dsa_common.h"

#include "stcp_connection.h"

#include <boost/asio/write.hpp>

#include "module/logger.h"
#include "util/little_endian.h"

namespace dsa {

StcpConnection::StcpConnection(const SharedLinkStrandRef &strand,
                               const string_ &dsid_prefix, const string_ &path)
    : BaseSocketConnection(strand, dsid_prefix, path) {}

void StcpConnection::destroy_impl() {
  LOG_DEBUG(__FILENAME__, LOG << "connection closed");
  _socket->lowest_layer().close();
  Connection::destroy_impl();
}

void StcpConnection::start_read(shared_ptr_<Connection> &&connection) {
  std::vector<uint8_t> &buffer = _read_buffer;
  size_t partial_size = _read_next - _read_current;
  if (_read_current > 0) {
    std::copy(buffer.data() + _read_current, buffer.data() + _read_next,
              buffer.data());
  }
  if (_read_next * 2 > buffer.size() && buffer.size() < MAX_BUFFER_SIZE) {
    buffer.resize(buffer.size() * 4);
  }
  _socket->async_read_some(
      boost::asio::buffer(&buffer[partial_size], buffer.size() - partial_size),
      [this, connection = std::move(connection), partial_size](
          const boost::system::error_code &err, size_t transferred) mutable {
        read_loop_(std::move(connection), partial_size, err, transferred);
      });
}

std::unique_ptr<ConnectionWriteBuffer> StcpConnection::get_write_buffer() {
  return std::unique_ptr<ConnectionWriteBuffer>(new WriteBuffer(*this));
}

size_t StcpConnection::WriteBuffer::max_next_size() const {
  return MAX_BUFFER_SIZE - size;
};

void StcpConnection::WriteBuffer::add(const Message &message, int32_t rid,
                                      int32_t ack_id) {
  size_t total_size = size + message.size();
  if (total_size > MAX_BUFFER_SIZE) {
    LOG_FATAL(__FILENAME__, LOG << "message is bigger than max buffer size: "
                                << MAX_BUFFER_SIZE);
  }

  while (total_size > connection._write_buffer.size()) {
    connection._write_buffer.resize(connection._write_buffer.size() * 4);
  }
  message.write(&connection._write_buffer[size], rid, ack_id);
  size += message.size();
}
void StcpConnection::WriteBuffer::write(WriteHandler &&callback) {
  std::lock_guard<std::mutex> read_loop_lock(connection.mutex);
  boost::asio::async_write(
      *connection._socket,
      boost::asio::buffer(connection._write_buffer.data(), size),
      [callback = std::move(callback)](const boost::system::error_code &error,
                                       size_t bytes_transferred) {
        DSA_REF_GUARD;

        callback(error);
      });
}

ssl_socket::lowest_layer_type &StcpConnection::socket() {
  return _socket->lowest_layer();
}

}  // namespace dsa

#include "dsa_common.h"

#include "tcp_connection.h"

#include <boost/asio/write.hpp>
#include <boost/thread/locks.hpp>

#include "tcp_client.h"
#include "tcp_server.h"

#include "util/little_endian.h"

namespace dsa {

TcpConnection::TcpConnection(LinkStrandRef &strand,
                             uint32_t handshake_timeout_ms,
                             const std::string &dsid_prefix,
                             const std::string &path)
    : Connection(strand, handshake_timeout_ms, dsid_prefix, path),
      _socket((*strand)()->get_io_service()) {}

void TcpConnection::close_impl() {
  LOG_DEBUG(_strand->logger(), LOG << "connection closed");
  if (_socket_open.exchange(false)) {
    _socket.close();
  }
  Connection::close_impl();
}

void TcpConnection::start_read(shared_ptr_<TcpConnection> &&connection,
                               size_t cur, size_t next) {
  std::vector<uint8_t> &buffer = connection->_read_buffer;
  size_t partial_size = next - cur;
  if (cur > 0) {
    std::copy(&buffer[cur], &buffer[next], &buffer[0]);
  } else if (partial_size * 2 > buffer.size() &&
             buffer.size() < MAX_BUFFER_SIZE) {
    // resize the buffer on demand
    buffer.resize(buffer.size() * 4);
  }
  tcp_socket &socket = connection->_socket;
  socket.async_read_some(
      boost::asio::buffer(&buffer[partial_size], buffer.size() - partial_size),
      [ connection = std::move(connection), partial_size ](
          const boost::system::error_code &err, size_t transferred) mutable {
        TcpConnection::read_loop(std::move(connection), partial_size, err,
                                 transferred);
      });
}

void TcpConnection::read_loop(shared_ptr_<TcpConnection> &&connection,
                              size_t from_prev,
                              const boost::system::error_code &error,
                              size_t bytes_transferred) {
  // reset deadline timer for each new message
  // TODO: make this thread safe
  // connection->reset_standard_deadline_timer();

  if (!error) {
    std::vector<uint8_t> &buffer = connection->_read_buffer;
    size_t total_bytes = from_prev + bytes_transferred;
    size_t cur = 0;
    {
      boost::lock_guard<boost::mutex> read_loop_lock(
          connection->read_loop_mutex);
      while (cur < total_bytes) {
        if (total_bytes - cur < sizeof(uint32_t)) {
          // not enough data to check size
          start_read(std::move(connection), cur, total_bytes);
          return;
        }
        // TODO: check if message_size is valid;
        int32_t message_size = read_32_t(&buffer[cur]);
        if (message_size > MAX_BUFFER_SIZE) {
          LOG_DEBUG(connection->_strand->logger(),
                    LOG << "message is bigger than maxed buffer size");
          TcpConnection::close_in_strand(connection);
          // TODO: send error, and close with std::move
          // TcpConnection::close_in_strand(std::move(connection));
          return;
        }
        if (message_size > total_bytes - cur) {
          // not enough data to parse message
          start_read(std::move(connection), cur, total_bytes);
          return;
        }

        // post job with message buffer

        if (connection->on_read_message != nullptr) {
          try {
            connection->on_read_message(
                Message::parse_message(&buffer[cur], message_size));
          } catch (const MessageParsingError &err) {
            LOG_DEBUG(connection->_strand->logger(),
                      LOG << "invalid message received, close connection : "
                          << err.what());
            TcpConnection::close_in_strand(connection);
            // TODO: send error, and close with std::move
            // TcpConnection::close_in_strand(std::move(connection));
            return;
          }

        } else {
          LOG_FATAL(connection->_strand->logger(),
                    LOG << "on_read_message is null");
        }

        cur += message_size;
      }
    }
    start_read(std::move(connection), 0, 0);
  } else {
    // TODO: send error
    TcpConnection::close_in_strand(std::move(connection));
    return;
  }
}

void TcpConnection::write(const uint8_t *data, size_t size,
                          WriteHandler &&callback) {
  boost::asio::async_write(
      _socket,
      boost::asio::buffer(data, size), [callback = std::move(callback)](
                                           const boost::system::error_code
                                               &error,
                                           size_t bytes_transferred) {
        callback(error);
      });
}

tcp_socket &TcpConnection::socket() { return _socket; }

}  // namespace dsa
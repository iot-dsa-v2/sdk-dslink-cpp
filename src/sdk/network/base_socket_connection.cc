#include "base_socket_connection.h"

#include "module/logger.h"
#include "util/little_endian.h"

namespace dsa {

BaseSocketConnection::BaseSocketConnection(LinkStrandRef &strand,
                                           const string_ &dsid_prefix,
                                           const string_ &path)
    : Connection(strand, dsid_prefix, path),
      _read_buffer(DEFAULT_BUFFER_SIZE),
      _write_buffer(DEFAULT_BUFFER_SIZE) {}

void BaseSocketConnection::on_deadline_timer_(
    const boost::system::error_code &error, shared_ptr_<Connection> &&sthis) {
  LOG_WARN(_strand->logger(), LOG << "Connection timeout");
  destroy_in_strand(std::move(sthis));
}

void BaseSocketConnection::read_loop_(shared_ptr_<Connection> &&connection,
                                      size_t from_prev,
                                      const boost::system::error_code &error,
                                      size_t bytes_transferred) {
  // reset deadline timer for each new message
  // TODO: make this thread safe
  // connection->reset_standard_deadline_timer();

  if (!error) {
    std::vector<uint8_t> &buffer = _read_buffer;
    size_t total_bytes = from_prev + bytes_transferred;
    size_t cur = 0;
    {
      std::lock_guard<std::mutex> read_loop_lock(mutex);
      if (is_destroyed()) {
        return;
      }

      while (cur < total_bytes) {
        if (total_bytes - cur < sizeof(uint32_t)) {
          // not enough data to check size
          start_read(std::move(connection), cur, total_bytes);
          return;
        }
        // TODO: check if message_size is valid;
        int32_t message_size = read_32_t(&buffer[cur]);
        if (message_size > Message::MAX_MESSAGE_SIZE) {
          LOG_DEBUG(_strand->logger(),
                    LOG << "message is bigger than maxed buffer size");
          destroy_in_strand(std::move(connection));
          // TODO: send error, and close with std::move
          // TcpConnection::destroy_in_strand(std::move(connection));
          return;
        }
        if (message_size > total_bytes - cur) {
          // not enough data to parse message
          start_read(std::move(connection), cur, total_bytes);
          return;
        }

        // post job with message buffer

        if (on_read_message != nullptr) {
          try {
            on_read_message(Message::parse_message(&buffer[cur], message_size));
          } catch (const MessageParsingError &err) {
            LOG_DEBUG(_strand->logger(),
                      LOG << "invalid message received, close connection : "
                          << err.what());
            destroy_in_strand(std::move(connection));
            // TODO: send error, and close with std::move
            // TcpConnection::destroy_in_strand(std::move(connection));
            return;
          }

        } else {
          LOG_FATAL(LOG << "on_read_message is null");
        }

        cur += message_size;
      }

      if (!_batch_post.empty()) {
        do_batch_post(std::move(connection));
        return;
      }
    }
    start_read(std::move(connection), 0, 0);
  } else {
    // TODO: send error
    destroy_in_strand(std::move(connection));
    return;
  }
}

}  // namespace dsa
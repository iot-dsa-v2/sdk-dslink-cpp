#include "dsa_common.h"

#include "ws_connection.h"

#include <boost/asio/write.hpp>

#include "module/logger.h"
#include "util/little_endian.h"

namespace dsa {

WsConnection::WsConnection(websocket_stream &ws, LinkStrandRef &strand,
                           const string_ &dsid_prefix, const string_ &path)
    : Connection(strand, dsid_prefix, path),
      _ws(std::move(ws)),
      _read_buffer(DEFAULT_BUFFER_SIZE),
      _write_buffer(DEFAULT_BUFFER_SIZE) {}

void WsConnection::on_deadline_timer_(const boost::system::error_code &error,
                                      shared_ptr_<Connection> sthis) {
  LOG_WARN(_strand->logger(), LOG << "Connection timeout");
  destroy_in_strand(std::move(sthis));
}

void WsConnection::destroy_impl() {
  /*
  LOG_DEBUG(_strand->logger(), LOG << "connection closed");
  if (_socket_open.exchange(false)) {
    _socket.close();
  }
  Connection::destroy_impl();
  */
}

void WsConnection::start_read(shared_ptr_<WsConnection> &&connection,
                              size_t cur, size_t next) {
  std::vector<uint8_t> &buffer = connection->_read_buffer;
  size_t partial_size = next - cur;
  if (cur > 0) {
    std::copy(buffer.data() + cur, buffer.data() + next, buffer.data());
  }
  if (next * 2 > buffer.size() && buffer.size() < MAX_BUFFER_SIZE) {
    buffer.resize(buffer.size() * 4);
  }
  websocket_stream &ws = connection->_ws;
  ws.async_read_some(
      boost::asio::buffer(&buffer[partial_size], buffer.size() - partial_size),
      [ this, connection = std::move(connection), partial_size ](
          const boost::system::error_code &err, size_t transferred) mutable {
        read_loop_(std::move(connection), partial_size, err, transferred);
      });
}

void WsConnection::read_loop_(shared_ptr_<WsConnection> &&connection,
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
      // connection post messages to main strand in batch
      // need a null message in the end to actually send all messages
      bool need_null_end = false;
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
          // WsConnection::destroy_in_strand(std::move(connection));
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
            need_null_end = on_read_message(
                Message::parse_message(&buffer[cur], message_size));
          } catch (const MessageParsingError &err) {
            LOG_DEBUG(_strand->logger(),
                      LOG << "invalid message received, close connection : "
                          << err.what());
            destroy_in_strand(std::move(connection));
            // TODO: send error, and close with std::move
            // WsConnection::destroy_in_strand(std::move(connection));
            return;
          }

        } else {
          LOG_FATAL(LOG << "on_read_message is null");
        }

        cur += message_size;
      }

      if (need_null_end) {
        on_read_message(MessageRef());
      }
    }
    start_read(std::move(connection), 0, 0);
  } else {
    // TODO: send error
    destroy_in_strand(std::move(connection));
    return;
  }
}

std::unique_ptr<ConnectionWriteBuffer> WsConnection::get_write_buffer() {
  return std::unique_ptr<ConnectionWriteBuffer>(new WriteBuffer(*this));
}

size_t WsConnection::WriteBuffer::max_next_size() const {
  return MAX_BUFFER_SIZE - size;
};

void WsConnection::WriteBuffer::add(const Message &message, int32_t rid,
                                    int32_t ack_id) {
  size_t total_size = size + message.size();
  if (total_size > connection._write_buffer.size()) {
    if (total_size <= MAX_BUFFER_SIZE) {
      connection._write_buffer.resize(connection._write_buffer.size() * 4);
    } else {
      LOG_FATAL(LOG << "message is bigger than max buffer size: "
                    << MAX_BUFFER_SIZE);
    }
  }
  message.write(&connection._write_buffer[size], rid, ack_id);
  size += message.size();
}
void WsConnection::WriteBuffer::write(WriteHandler &&callback) {
  connection._ws.binary(true);
  connection._ws.async_write(
      boost::asio::buffer(connection._write_buffer.data(), size),
      [callback = std::move(callback)](const boost::system::error_code &error,
                                       size_t bytes_transferred) {
        callback(error);
      });
}

websocket_stream &WsConnection::websocket() { return _ws; }

}  // namespace dsa

#include "dsa_common.h"

#include "connection.h"

#include "core/app.h"
#include "core/session.h"
#include "crypto/hmac.h"

#define DEBUG 0

namespace dsa {
Connection::Connection(LinkStrandPtr &strand, uint32_t handshake_timeout_ms,
                       const std::string &dsid_prefix, const std::string &path)
    : _handshake_context(dsid_prefix, strand->ecdh()),
      _handshake_timeout_ms(handshake_timeout_ms),
      _read_buffer(DEFAULT_BUFFER_SIZE),
      _write_buffer(DEFAULT_BUFFER_SIZE),
      _deadline((*strand)().get_io_service()),
      _strand(strand),
      _path(path) {}

void Connection::connect() { throw std::runtime_error("not implemented"); }
void Connection::accept() { throw std::runtime_error("not implemented"); }

void Connection::set_session(const intrusive_ptr_<Session> &session) {
  _session = session;
}

void Connection::success_or_close(const boost::system::error_code &error) {
  if (error != nullptr) close();
}

void Connection::close_impl() {
  if (_session != nullptr) {
    _session->connection_closed();
    _session.reset();
  }
  _deadline.cancel();
}

void Connection::close_in_strand(shared_ptr_<Connection> &&connection) {
  Connection *raw_conn = connection.get();
  // obtain the lock before dispatch to strand to reduce the load on main strand

  (*raw_conn->_strand)().dispatch([
    connection = std::move(connection),
    // because asio require callback to be copy constructable, lock can't be
    // directly used, and we need shared_ptr
    lock = std::make_shared<boost::unique_lock<boost::shared_mutex>>(
        raw_conn->mutex)
  ]() mutable { connection->close(*lock); });
}

void Connection::compute_secret() {
  _shared_secret = _handshake_context.ecdh().compute_secret(_other_public_key);

  /* compute user auth */
  dsa::HMAC hmac("sha256", _shared_secret);
  hmac.update(_other_salt);
  _auth = hmac.digest();

  /* compute other auth */
  dsa::HMAC other_hmac("sha256", _shared_secret);
  other_hmac.update(_handshake_context.salt());
  _other_auth = other_hmac.digest();

#if DEBUG
  std::stringstream ss;
  ss << name() << "::compute_secret()" << std::endl;
  ss << "auth:       " << *_auth << std::endl;
  ss << "other auth: " << *_other_auth << std::endl;
  std::cout << ss.str();
#endif
}

bool Connection::valid_handshake_header(StaticHeaders &header,
                                        size_t expected_size,
                                        MessageType expected_type) {
  return (header.message_size == expected_size &&
          header.header_size == StaticHeaders::TotalSize &&
          header.type == expected_type && header.request_id == 0 &&
          header.ack_id == 0);
}

void Connection::reset_standard_deadline_timer() {
  _deadline.expires_from_now(boost::posix_time::minutes(1));
  _deadline.async_wait((*_strand)().wrap([sthis = shared_from_this()](
      const boost::system::error_code &error) {
    if (error != boost::asio::error::operation_aborted) {
      sthis->close();
    }
  }));
}

void Connection::post_message(Message *message) {
  if (_session != nullptr) {
    _session->receive_message(message);
  } else {
    delete message;
  }
}
}  // namespace dsa
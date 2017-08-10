#include "dsa_common.h"

#include "connection.h"
#include "core/session.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "core/app.h"
#include "crypto/hmac.h"

#define DEBUG 0

namespace dsa {

Connection::Connection(boost::asio::io_service::strand &strand, const Config &config)
    : _handshake_context(config.dsid_prefix, config.ecdh),
      _read_buffer(new Buffer()),
      _write_buffer(new Buffer()),
      _config(config),
      _deadline(strand.get_io_service()),
      _strand(strand) {}

void Connection::success_or_close(const boost::system::error_code &error) {
  if (error != nullptr) close();
}

void Connection::close() {
  if (_session != nullptr) {
    _session->connection_closed();
    _session.reset();
  }
  _deadline.cancel();
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

bool Connection::valid_handshake_header(StaticHeaders &header, size_t expected_size, MessageType expected_type) {
  return (
      header.message_size == expected_size &&
          header.header_size == StaticHeaders::TotalSize &&
          header.type == expected_type &&
          header.request_id == 0 &&
          header.ack_id == 0
  );
}

void Connection::timeout(const boost::system::error_code &error) {
  if (error != boost::asio::error::operation_aborted) {
    close();
  }
}

void Connection::reset_standard_deadline_timer() {
  _deadline.expires_from_now(boost::posix_time::minutes(1));
  _deadline.async_wait(boost::bind(&Connection::timeout, shared_from_this(), boost::asio::placeholders::error));
}

}  // namespace dsa
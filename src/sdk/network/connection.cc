#include "dsa_common.h"

#include "connection.h"

#include "core/app.h"
#include "core/session.h"
#include "crypto/hmac.h"

#include <boost/asio/strand.hpp>

namespace dsa {
Connection::Connection(LinkStrandRef &strand, uint32_t handshake_timeout_ms,
                       const std::string &dsid_prefix, const std::string &path)
    : _handshake_context(dsid_prefix, strand->ecdh()),
      _handshake_timeout_ms(handshake_timeout_ms),
      _read_buffer(DEFAULT_BUFFER_SIZE),
      _write_buffer(DEFAULT_BUFFER_SIZE),
      _deadline(static_cast<boost::asio::strand *>(strand->asio_strand())
                    ->get_io_service()),
      _strand(strand),
      _path(path) {}

void Connection::connect() { throw std::runtime_error("not implemented"); }
void Connection::accept() { throw std::runtime_error("not implemented"); }

void Connection::set_session(const ref_<Session> &session) {
  _session = session;
}

void Connection::success_or_close(const boost::system::error_code &error) {
  if (error != nullptr) close();
}

void Connection::close_impl() {
  if (_session != nullptr) {
    _session->disconnected(shared_from_this());
    _session.reset();
  }
  _deadline.cancel();
}

bool Connection::valid_handshake_header(StaticHeaders &header,
                                        size_t expected_size,
                                        MessageType expected_type) {
  return (header.message_size == expected_size &&
          header.header_size == StaticHeaders::TOTAL_SIZE &&
          header.type == expected_type && header.rid == 0 &&
          header.ack_id == 0);
}

void Connection::reset_standard_deadline_timer() {
  //  _deadline.expires_from_now(boost::posix_time::minutes(1));
  //  _deadline.async_wait((*_strand)()->wrap([sthis = shared_from_this()](
  //      const boost::system::error_code &error) {
  //    if (error != boost::asio::error::operation_aborted) {
  //      sthis->close();
  //    }
  //  }));
}

void Connection::post_message(MessageRef &&message) {
  if (_session != nullptr) {
    _strand->post(
        [ sthis = shared_from_this(), message = std::move(message) ]() mutable {
          if (sthis->session() != nullptr) {
            sthis->session()->receive_message(std::move(message));
          }
        });
  }
}
}  // namespace dsa
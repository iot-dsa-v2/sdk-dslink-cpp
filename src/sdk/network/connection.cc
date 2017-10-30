#include "dsa_common.h"

#include "connection.h"

#include "core/app.h"
#include "core/session.h"
#include "crypto/hmac.h"

namespace dsa {
Connection::Connection(LinkStrandRef &strand,
                       const string_ &dsid_prefix, const string_ &path)
    : _handshake_context(dsid_prefix, strand->ecdh()),
      _deadline(strand->get_io_service()),
      _strand(strand),
      _path(path) {}

void Connection::connect() { throw std::runtime_error("not implemented"); }
void Connection::accept() { throw std::runtime_error("not implemented"); }

void Connection::set_session(const ref_<Session> &session) {
  _session = session;
}

void Connection::success_or_close(const boost::system::error_code &error) {
  if (error != nullptr) destroy();
}

void Connection::destroy_impl() {
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

bool Connection::post_message(MessageRef &&message) {
  if (message == nullptr) {
    if (_session != nullptr && !_batch_post.empty()) {
      std::vector<MessageRef> copy;
      _batch_post.swap(copy);
      _strand->post(
        [ sthis = shared_from_this(), messages = std::move(copy) ]() mutable {
          if (sthis->session() != nullptr) {
            auto session = sthis->session();
            for (auto & it : messages) {
              session->receive_message(std::move(it));
            }
          }
        });
    }
    return false;
  } else {
    _batch_post.emplace_back(std::move(message));
    return true;
  }
}
}  // namespace dsa
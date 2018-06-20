#include "dsa_common.h"

#include "connection.h"

#include "core/session.h"
#include "crypto/hmac.h"
#include "module/logger.h"
#include "util/app.h"
#include "util/string.h"

namespace dsa {

bool Connection::_requester_auth_key_inited = false;
std::vector<uint8_t> Connection::_requester_auth_key;

const std::vector<uint8_t> &Connection::get_requester_auth_key() {
  if (!_requester_auth_key_inited) {
    _requester_auth_key_inited = true;
    auto auth_key_str =
        string_from_storage("requester-auth-key", Storage::get_config_bucket());
    // convert string back to vector
    _requester_auth_key.assign(auth_key_str.begin(), auth_key_str.end());
  }
  return _requester_auth_key;
}
Connection::Connection(const SharedLinkStrandRef &strand,
                       const string_ &dsid_prefix, const string_ &path)
    : _handshake_context(dsid_prefix, strand->get_ecdh()),
      _deadline(strand->get_io_context()),
      _shared_strand(strand),
      _path(path) {}

void Connection::post_in_strand(std::function<void()> &&callback,
                                bool already_locked) {
  if (already_locked) {
    if (_shared_strand != nullptr) {
      return _shared_strand->post(std::move(callback));
    }
  } else {
    std::lock_guard<std::mutex> unique_lock(mutex);
    if (_shared_strand != nullptr) {
      return _shared_strand->post(std::move(callback));
    }
  }
}

void Connection::connect(size_t reconnect_interval) {
  throw std::runtime_error("not implemented");
}
void Connection::accept() { throw std::runtime_error("not implemented"); }

void Connection::start_deadline_timer(size_t seconds) {
  if (seconds > 0) {
    _deadline.expires_from_now(boost::posix_time::seconds(seconds));
    _deadline.async_wait([sthis = shared_from_this()](
                             const boost::system::error_code &error) mutable {
      if (error != boost::asio::error::operation_aborted) {
        sthis->on_deadline_timer_(error, std::move(sthis));
      }
    });
  }
}
void Connection::reset_deadline_timer(size_t seconds) {
  _deadline.expires_from_now(boost::posix_time::seconds(seconds));
}
void Connection::set_session(const ref_<Session> &session) {
  _session = session;
}

void Connection::destroy_impl() {
  _deadline.cancel();
  _shared_strand.reset();
  if (_session != nullptr) {
    // it's possibel that this will be deleted here
    // so reset the smart pointer after this line is not safe
    remove_ref_(_session)->disconnected(shared_from_this());
  }
}

void Connection::post_message(MessageRef &&message) {
  _batch_post.emplace_back(std::move(message));
}
void Connection::do_batch_post(shared_ptr_<Connection> &&sthis) {
  if (_session != nullptr && !_batch_post.empty()) {
    std::vector<MessageRef> copy;
    _batch_post.swap(copy);
    _shared_strand->post(
        [this, sthis = std::move(sthis), messages = std::move(copy)](
            ref_<LinkStrand> &, LinkStrand &strand) mutable {
          if (is_destroyed()) return;
          // a special protection to give writing higher priority than reading
          strand.check_injected();

          continue_read_loop(std::move(sthis));
          if (_session != nullptr) {
            for (auto &it : messages) {
              _session->receive_message(std::move(it));
            }
          }
        });
  } else if (_session == nullptr) {
    LOG_ERROR(__FILENAME__, "Session is null in connection message reading");
  }
}
}  // namespace dsa

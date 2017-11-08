#include "dsa_common.h"

#include "connection.h"

#include "core/server.h"
#include "message/handshake/f0_message.h"
#include "message/handshake/f1_message.h"
#include "message/handshake/f2_message.h"
#include "message/handshake/f3_message.h"
#include "module/logger.h"

#include "core/session_manager.h"

namespace dsa {

bool Connection::on_receive_f0(MessageRef &&msg) {
  if (msg->type() != MessageType::HANDSHAKE0) {
    throw MessageParsingError("invalid handshake message, expect f0");
  }
  LOG_DEBUG(_strand->logger(), LOG << "f0 received");
  auto *f0 = DOWN_CAST<HandshakeF0Message *>(msg.get());
  this->_handshake_context.set_remote(
      std::move(f0->dsid), std::move(f0->public_key), std::move(f0->salt));

  HandshakeF1Message f1;
  f1.dsid = _handshake_context.dsid();
  f1.public_key = _handshake_context.public_key();
  f1.salt = _handshake_context.salt();
  f1.size();  // calculate size
  auto write_buffer = get_write_buffer();
  write_buffer->add(f1, 0, 0);

  write_buffer->write([ this, sthis = shared_from_this() ](
      const boost::system::error_code &err) mutable {
    if (err != boost::system::errc::success) {
      destroy_in_strand(std::move(sthis));
    }
  });

  // no need to lock, parent scope should already have the lock
  on_read_message = [this](MessageRef message) {
    return on_receive_f2(std::move(message));
  };
  return false;
}

bool Connection::on_receive_f2(MessageRef &&msg) {
  if (msg->type() != MessageType::HANDSHAKE2) {
    throw MessageParsingError("invalid handshake message, expect f2");
  }
  LOG_DEBUG(_strand->logger(), LOG << "f2 received");

  auto *f2 = DOWN_CAST<HandshakeF2Message *>(msg.get());

  _handshake_context.compute_secret();

  if (std::equal(_handshake_context.remote_auth().begin(),
                 _handshake_context.remote_auth().end(), f2->auth.begin())) {
    _strand->post([ msg, this, sthis = shared_from_this() ]() mutable {
      auto *f2 = DOWN_CAST<HandshakeF2Message *>(msg.get());
      _strand->session_manager().get_session(
          _handshake_context.remote_dsid(), f2->token, f2->previous_session_id,
          [ this, sthis = std::move(sthis) ](const ref_<Session> &session) {
            if (session != nullptr) {
              _session = session;
              _session->connected(shared_from_this());

              std::lock_guard<std::mutex> lock(mutex);
              on_read_message = [this](MessageRef message) {
                return post_message(std::move(message));
              };

              HandshakeF3Message f3;
              f3.auth = _handshake_context.auth();
              f3.session_id = _session->session_id();

              f3.size();  // calculate size
              auto write_buffer = get_write_buffer();
              write_buffer->add(f3, 0, 0);

              write_buffer->write([ this, sthis = shared_from_this() ](
                  const boost::system::error_code &err) mutable {
                if (err != boost::system::errc::success) {
                  destroy_in_strand(std::move(sthis));
                }
              });
            } else {
              destroy();
            }
          });

    });

  } else {
    Connection::destroy_in_strand(shared_from_this());
  }
  return false;
}
}

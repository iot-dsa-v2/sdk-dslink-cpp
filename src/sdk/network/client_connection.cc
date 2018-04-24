#include "dsa_common.h"

#include "connection.h"

#include "core/client.h"
#include "core/session.h"
#include "crypto/misc.h"
#include "message/handshake/f0_message.h"
#include "message/handshake/f1_message.h"
#include "message/handshake/f2_message.h"
#include "message/handshake/f3_message.h"
#include "module/logger.h"

namespace dsa {

void Connection::on_client_connect(shared_ptr_<Connection> connection) throw(
    const std::runtime_error &) {
  if (connection->_session == nullptr) {
    LOG_FATAL(__FILENAME__, LOG << "no session attached to client connection");
  }
  Connection *raw_ptr = connection.get();
  raw_ptr->_session->connected(std::move(connection));
}
void Connection::start_client_f0() {
  HandshakeF0Message f0;
  f0.dsid = _handshake_context.dsid();
  f0.public_key = _handshake_context.public_key();
  f0.salt = _handshake_context.salt();
  f0.size();  // calculate size
  auto write_buffer = get_write_buffer();
  write_buffer->add(f0, 0, 0);

  write_buffer->write([ this, sthis = shared_from_this() ](
      const boost::system::error_code &err) mutable {
    if (err != boost::system::errc::success) {
      destroy_in_strand(std::move(sthis));
    }
  });
  // wait another 15 seconds until the connection timeout
  reset_deadline_timer(15);
  std::lock_guard<std::mutex> lock(mutex);
  on_read_message = [this](MessageRef message) {
    on_receive_f1(std::move(message));
  };
}
void Connection::on_receive_f1(MessageRef &&msg) {
  if (msg->type() != MessageType::HANDSHAKE1) {
    throw MessageParsingError("invalid handshake message, expect f1");
  }
  LOG_DEBUG(__FILENAME__, LOG << "f1 received");
  auto *f1 = DOWN_CAST<HandshakeF1Message *>(msg.get());
  _handshake_context.set_remote(std::move(f1->dsid), std::move(f1->public_key),
                                std::move(f1->salt));
  _handshake_context.compute_secret();

  HandshakeF2Message f2;
  f2.auth = _handshake_context.auth();
  f2.is_responder = _session->responder_enabled;
  if (_session->client_token.length() > 16) {
    f2.token =
        generate_auth_token(_handshake_context.dsid(), _session->client_token);
  }

  auto write_buffer = get_write_buffer();
  write_buffer->add(f2, 0, 0);

  write_buffer->write([ this, sthis = shared_from_this() ](
      const boost::system::error_code &err) mutable {
    if (err != boost::system::errc::success) {
      destroy_in_strand(std::move(sthis));
    }
  });

  // wait another 15 seconds until the connection timeout
  reset_deadline_timer(15);
  // no need to lock, parent scope should already have the lock
  on_read_message = [this](MessageRef message) {
    on_receive_f3(std::move(message));
  };
}

void Connection::on_receive_f3(MessageRef &&msg) {
  if (msg->type() != MessageType::HANDSHAKE3) {
    throw MessageParsingError("invalid handshake message, expect f3");
  }
  LOG_DEBUG(__FILENAME__, LOG << "f3 received ");

  auto *f3 = DOWN_CAST<HandshakeF3Message *>(msg.get());

  if (std::equal(_handshake_context.remote_auth().begin(),
                 _handshake_context.remote_auth().end(), f3->auth.begin())) {
    _deadline.cancel();
    _remote_path = f3->path;

    _shared_strand->post([sthis = shared_from_this()]() mutable {
      on_client_connect(std::move(sthis));
    });
    on_read_message = [this](MessageRef &&message) {
      post_message(std::move(message));
    };
  } else {
    LOG_ERROR(__FILENAME__, LOG << "invalid handshake auth");
  }
}

}  // namespace dsa

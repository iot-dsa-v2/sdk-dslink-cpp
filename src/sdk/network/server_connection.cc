#include "dsa_common.h"

#include "connection.h"

#include "core/server.h"
#include "message/handshake/f0_message.h"
#include "message/handshake/f1_message.h"
#include "message/handshake/f2_message.h"
#include "message/handshake/f3_message.h"
#include "module/logger.h"
#include "stream/simple_stream.h"

#include "module/session_manager.h"

namespace dsa {

void Connection::on_receive_f0(MessageRef &&msg) {
  if (msg->type() != MessageType::HANDSHAKE0) {
    throw MessageParsingError("invalid handshake message, expect f0");
  }
  LOG_DEBUG(__FILENAME__, LOG << "f0 received");
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

  // wait another 15 seconds until the connection timeout
  reset_deadline_timer(15);
  // no need to lock, parent scope should already have the lock
  on_read_message = [this](MessageRef message) {
    on_receive_f2(std::move(message));
  };
}

void Connection::on_receive_f2(MessageRef &&msg) {
  if (msg->type() != MessageType::HANDSHAKE2) {
    throw MessageParsingError("invalid handshake message, expect f2");
  }
  LOG_DEBUG(__FILENAME__, LOG << "f2 received");

  auto *f2 = DOWN_CAST<HandshakeF2Message *>(msg.get());

  _handshake_context.compute_secret();

  if (std::equal(_handshake_context.remote_auth().begin(),
                 _handshake_context.remote_auth().end(), f2->auth.begin())) {
    _deadline.cancel();
    _remote_path = f2->path;
    _shared_strand->post([
      msg, this, sthis = shared_from_this(), is_responder = f2->is_responder
    ](ref_<LinkStrand>&, LinkStrand& strand) mutable {
      auto *f2 = DOWN_CAST<HandshakeF2Message *>(msg.get());
      strand.session_manager().get_session(
          _handshake_context.remote_dsid(), f2->token, is_responder,
          [ this, sthis = std::move(sthis) ](const ref_<Session> &session,
                                             const ClientInfo &info) {
            if (session != nullptr) {
              _session = session;

              // send f3, now session owns the write loop
              // can't send it with raw buffer
              auto f3 = make_ref_<HandshakeF3Message>();
              f3->auth = _handshake_context.auth();
              f3->path = info.responder_path;
              _session->write_critical_stream(
                  make_ref_<SimpleStream>(0, std::move(f3)));

              // connected after adding f3 stream to the front of queue
              // this makes sure nothing get sent before f3
              _session->connected(shared_from_this());

              {
                std::lock_guard<std::mutex> lock(mutex);
                on_read_message = [this](MessageRef &&message) {
                  post_message(std::move(message));
                };
              }
            } else {
              destroy();
            }
          });

    });

  } else {
    Connection::destroy_in_strand(shared_from_this());
  }
}
}  // namespace dsa

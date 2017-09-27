#include "dsa_common.h"

#include "connection.h"

#include "core/server.h"
#include "module/logger.h"
#include "message/handshake/f0_message.h"
#include "message/handshake/f1_message.h"
#include "message/handshake/f2_message.h"
#include "message/handshake/f3_message.h"

#include "core/session_manager.h"

namespace dsa {

// void Connection::on_server_connect() throw(const std::runtime_error &) {
//  // setup session now that client session id has been parsed
//  _strand->session_manager().get_session(
//      _other_dsid, _other_token, _session_id,
//      [=](const ref_<Session> &session) {
//        if (session != nullptr) {
//          _session = session;
//          _session_id = _session->session_id();
//          _session->set_connection(shared_from_this());
//          _session->start();
//        } else {
//          // TODO: send error
//        }
//      });
//}

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
  f1.write(_write_buffer.data());

  write(_write_buffer.data(),
        f1.size(), [sthis = shared_from_this()](
                       const boost::system::error_code &err) mutable {
          if (err != boost::system::errc::success) {
            Connection::close_in_strand(std::move(sthis));
          }
        });

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
          [ this,
            sthis = std::move(sthis) ](const ref_<Session> &session) {
            if (session != nullptr) {
              _session = session;
              _session->connected(shared_from_this());
              on_read_message = [this](MessageRef message) {
                return post_message(std::move(message));
              };
              
              HandshakeF3Message f3;
              f3.auth = _handshake_context.auth();
              f3.session_id = _session->session_id();

              f3.size();  // calculate size
              f3.write(_write_buffer.data());

              write(
                  _write_buffer.data(),
                  f3.size(), [sthis = shared_from_this()](
                                 const boost::system::error_code &err) mutable {
                    if (err != boost::system::errc::success) {
                      Connection::close_in_strand(std::move(sthis));
                    }
                  });
              boost::lock_guard<boost::mutex> read_loop_lock(read_loop_mutex);


            } else {
              close();
            }
          });

    });

  } else {
    Connection::close_in_strand(shared_from_this());
  }
  return false;
}

/////////////////////////_write_buffer->data
// bool Connection::parse_f0(size_t size) {
//  if (size < MinF0Length) return false;
//
//  const uint8_t *data = _write_buffer.data();
//
//  StaticHeaders header(data);
//
//  if (!valid_handshake_header(header, size, MessageType::HANDSHAKE0))
//    return false;
//
//  data += StaticHeaders::TotalSize;
//  uint8_t dsid_length;
//
//  //  _dsa_version_major = *data++;
//  //  _dsa_version_minor = *data++;
//  dsid_length = *data++;
//
//  if ((data - _write_buffer.data()) + dsid_length + PUBLIC_KEY_LENGTH + 1 +
//          SALT_LENGTH >
//      size)
//    return false;
//
//  data += _other_dsid.assign(reinterpret_cast<const char *>(data),
//  dsid_length)
//              .size();
//  _other_public_key.assign(data, data + PUBLIC_KEY_LENGTH);
//  data += _other_public_key.size();
//
//  _security_preference = static_cast<bool>(*data++);
//
//  _other_salt.assign(data, data + SALT_LENGTH);
//
//  data += _other_salt.size();
//
//  return data == _write_buffer.data() + size;
//}
//
// bool Connection::parse_f2(size_t size) {
//  if (size < MinF2Length) return false;
//
//  const uint8_t *data = _write_buffer.data();
//
//  StaticHeaders header(data);
//  if (!valid_handshake_header(header, size, MessageType::HANDSHAKE2))
//    return false;
//
//  data += StaticHeaders::TotalSize;
//  uint16_t token_length;
//  uint16_t session_id_length;
//
//  std::copy(data, data + sizeof(token_length), &token_length);
//  data += sizeof(token_length);
//
//  // prevent accidental read in unowned memory
//  if ((data - _write_buffer.data()) + token_length + 2 +
//          sizeof(session_id_length) >
//      size)
//    return false;
//
//  data +=
//      _other_token.assign(reinterpret_cast<const char *>(data), token_length)
//          .size();
//  _is_requester = (*data++ != 0u);
//  _is_responder = (*data++ != 0u);
//  std::copy(data, data + sizeof(session_id_length), &session_id_length);
//  data += sizeof(session_id_length);
//
//  // prevent accidental read in unowned memory
//  if ((data - _write_buffer.data()) + session_id_length + AUTH_LENGTH != size)
//    return false;
//
//  data += _session_id
//              .assign(reinterpret_cast<const char *>(data), session_id_length)
//              .size();
//  _other_auth.assign(data, data + AUTH_LENGTH);
//  data += _other_auth.size();
//
//  return data == _write_buffer.data() + size;
//}
//
// size_t Connection::load_f1(std::vector<uint8_t> &buf) {
//  uint16_t dsid_length =
//      static_cast<uint16_t>(_handshake_context.dsid().size());
//
//  // ensure buf is large enough
//  buf.resize(MinF1Length + dsid_length);
//
//  // leave message size blank for now
//  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::HANDSHAKE1,
//  0,
//                       0);
//  uint8_t *data = buf.data();
//  header.write(data);
//  data += StaticHeaders::TotalSize;
//  data = std::copy(&dsid_length, &dsid_length + sizeof(dsid_length), data);
//  data += _handshake_context.dsid().copy(reinterpret_cast<char *>(data),
//                                         dsid_length);
//  data = std::copy(_handshake_context.public_key().begin(),
//                   _handshake_context.public_key().end(), data);
//  data = std::copy(_handshake_context.salt().begin(),
//                   _handshake_context.salt().end(), data);
//
//  uint32_t size = data - buf.data();
//  std::copy(&size, &size + sizeof(size), buf.data());
//
//  return size;
//}
//
// size_t Connection::load_f3(std::vector<uint8_t> &buf) {
//  auto sid_length = (uint16_t)_session_id.size();
//  auto path_length = (uint16_t)_path.size();
//
//  // ensure buf is large enough
//  buf.resize(MinF2Length + sid_length);
//
//  // leave message size blank for now
//  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake3,
//  0,
//                       0);
//  uint8_t *data = buf.data();
//  header.write(data);
//  data += StaticHeaders::TotalSize;
//  data = std::copy(&sid_length, &sid_length + sizeof(sid_length), data);
//  data += _session_id.copy(reinterpret_cast<char *>(data), sid_length);
//  data = std::copy(&path_length, &path_length + sizeof(path_length), data);
//  data += _path.copy(reinterpret_cast<char *>(data), path_length);
//  data = std::copy(_auth.begin(), _auth.end(), data);
//
//  uint32_t size = data - buf.data();
//  std::copy(&size, &size + sizeof(size), buf.data());
//
//  return size;
//}
}  // namespace dsa

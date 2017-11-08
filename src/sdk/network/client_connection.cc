#include "dsa_common.h"

#include "connection.h"

#include "core/client.h"
#include "core/session.h"
#include "message/handshake/f0_message.h"
#include "message/handshake/f1_message.h"
#include "message/handshake/f2_message.h"
#include "message/handshake/f3_message.h"
#include "module/logger.h"

namespace dsa {

void Connection::on_client_connect(shared_ptr_<Connection> connection) throw(
    const std::runtime_error &) {
  if (connection->_session == nullptr) {
    LOG_FATAL(LOG << "no session attached to client connection");
  }
  Connection *raw_ptr = connection.get();
  raw_ptr->_session->connected(std::move(connection));
  raw_ptr->on_read_message = [raw_ptr](MessageRef message) {
    return raw_ptr->post_message(std::move(message));
  };
}
void Connection::start_client_f0() {
  HandshakeF0Message f0;
  f0.dsid = _handshake_context.dsid();
  f0.public_key = _handshake_context.public_key();
  f0.salt = _handshake_context.salt();
  f0.size();  // calculate size
  auto write_buffer = get_write_buffer();
  write_buffer->add(f0, 0, 0);

  write_buffer->write([this, sthis = shared_from_this()](
      const boost::system::error_code &err) mutable {
    if (err != boost::system::errc::success) {
      destroy_in_strand(std::move(sthis));
    }
  });
  std::lock_guard<std::mutex> lock(mutex);
  on_read_message = [this](MessageRef message) {
    return on_receive_f1(std::move(message));
  };
}
bool Connection::on_receive_f1(MessageRef &&msg) {
  if (msg->type() != MessageType::HANDSHAKE1) {
    throw MessageParsingError("invalid handshake message, expect f1");
  }
  LOG_DEBUG(_strand->logger(), LOG << "f1 received");
  auto *f1 = DOWN_CAST<HandshakeF1Message *>(msg.get());
  _handshake_context.set_remote(std::move(f1->dsid), std::move(f1->public_key),
                                std::move(f1->salt));
  _handshake_context.compute_secret();

  HandshakeF2Message f2;
  f2.auth = _handshake_context.auth();
  // TODO:
  // f2.is_requester = true;
  // f2.is_responder = true;
  // f2.session_id =
  // f2.token =

  auto write_buffer = get_write_buffer();
  write_buffer->add(f2, 0, 0);

  write_buffer->write([this, sthis = shared_from_this()](
                       const boost::system::error_code &err) mutable {
          if (err != boost::system::errc::success) {
            destroy_in_strand(std::move(sthis));
          }
        });

  on_read_message = [this](MessageRef message) {
    return on_receive_f3(std::move(message));
  };
  return false;
}

bool Connection::on_receive_f3(MessageRef &&msg) {
  if (msg->type() != MessageType::HANDSHAKE3) {
    throw MessageParsingError("invalid handshake message, expect f3");
  }
  LOG_DEBUG(_strand->logger(), LOG << "f3 received");

  auto *f3 = DOWN_CAST<HandshakeF3Message *>(msg.get());

  if (std::equal(_handshake_context.remote_auth().begin(),
                 _handshake_context.remote_auth().end(), f3->auth.begin())) {
    _strand->post([sthis = shared_from_this()]() mutable {
      on_client_connect(std::move(sthis));
    });
  }
  return false;
}

/////////////////////////
//// Handshake Functions
/////////////////////////
//
// bool Connection::parse_f1(size_t size) {
//  if (size < MinF1Length) return false;
//
//  const uint8_t *data = _write_buffer.data();
//
//  StaticHeaders header(data);
//  if (!valid_handshake_header(header, size, MessageType::HANDSHAKE1))
//    return false;
//
//  data += StaticHeaders::TotalSize;
//  uint8_t dsid_length;
//
//  std::copy(data, data + sizeof(dsid_length), &dsid_length);
//  data += sizeof(dsid_length);
//
//  if ((data - _write_buffer.data()) + dsid_length + PUBLIC_KEY_LENGTH +
//          SALT_LENGTH >
//      size)
//    return false;
//
//  data += _other_dsid.assign(reinterpret_cast<const char *>(data),
//  dsid_length)
//              .size();
//
//  _other_public_key.assign(data, data + PUBLIC_KEY_LENGTH);
//  data += _other_public_key.size();
//
//  _other_salt.assign(data, data + SALT_LENGTH);
//  data += _other_salt.size();
//
//  return data == _write_buffer.data() + size;
//}
//
// bool Connection::parse_f3(size_t size) {
//  if (size < MinF3Length) return false;
//
//  const uint8_t *data = _write_buffer.data();
//
//  StaticHeaders header(data);
//  if (!valid_handshake_header(header, size, MessageType::HANDSHAKE3))
//    return false;
//
//  uint32_t cur = StaticHeaders::TotalSize;
//  uint16_t session_id_length, path_length;
//
//  std::copy(data, data + sizeof(session_id_length), &session_id_length);
//  data += sizeof(session_id_length);
//  if (cur + session_id_length + sizeof(path_length) > size) return false;
//  data += _session_id
//              .assign(reinterpret_cast<const char *>(data), session_id_length)
//              .size();
//
//  std::copy(data, data + sizeof(path_length), &path_length);
//  data += sizeof(path_length);
//  if (cur + path_length + AUTH_LENGTH > size) return false;
//  data +=
//      _path.assign(reinterpret_cast<const char *>(data), path_length).size();
//
//  _auth_check.assign(data, data + AUTH_LENGTH);
//  data += _auth_check.size();
//
//#if DEBUG
//  std::stringstream ss;
//  ss << name() << "::client_parse_f3()" << std::endl;
//  ss << "auth check: " << *auth_check << std::endl;
//  ss << "other auth: " << *_other_auth << std::endl;
//  std::cout << ss.str();
//#endif
//
//  // make sure other auth is correct
//  if (_auth_check != _other_auth) return false;
//
//  return data == _write_buffer.data() + size;
//}
//
//// Handshake load functions
// size_t Connection::load_f0(std::vector<uint8_t> &buf) {
//  uint16_t dsid_length =
//      static_cast<uint16_t>(_handshake_context.dsid().size());
//
//  // ensure buf is large enough
//  buf.resize(MinF0Length + _handshake_context.dsid().size());
//
//  // leave message size blank for now
//  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::HANDSHAKE0,
//  0,
//                       0);
//  uint8_t *data = buf.data();
//  header.write(data);
//  data += StaticHeaders::TotalSize;
//  (*data++) = (uint8_t)2;  // version major
//  (*data++) = (uint8_t)0;  // version major
//  data = std::copy(&dsid_length, &dsid_length + sizeof(dsid_length), data);
//  data += _handshake_context.dsid().copy(reinterpret_cast<char *>(data),
//                                         dsid_length);
//  data = std::copy(_handshake_context.public_key().begin(),
//                   _handshake_context.public_key().end(), data);
//  (*data++) = (uint8_t)0;  // no encryption for now
//  // TODO: implement encryption
//  data = std::copy(_handshake_context.salt().begin(),
//                   _handshake_context.salt().end(), data);
//
//  uint32_t size = data - buf.data();
//  std::copy(&size, &size + sizeof(size), buf.data());
//
//  return size;
//}
//
// size_t Connection::load_f2(std::vector<uint8_t> &buf) {
//  auto token_length = (uint16_t)_client_token.size();
//  auto sid_length = (uint16_t)_previous_session_id.size();
//
//  // ensure buf is large enough
//  buf.resize(MinF2Length + token_length);
//
//  // leave message size blank for now
//  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::HANDSHAKE2,
//  0,
//                       0);
//  uint8_t *data = buf.data();
//  header.write(data);
//  data += StaticHeaders::TotalSize;
//  data = std::copy(&token_length, &token_length + sizeof(token_length), data);
//  data += _client_token.copy(reinterpret_cast<char *>(data), token_length);
//  (*data++) = (uint8_t)(_is_requester ? 1 : 0);
//  (*data++) = (uint8_t)(_is_responder ? 1 : 0);
//  data = std::copy(&sid_length, &sid_length + sizeof(sid_length), data);
//  if (sid_length > 0) {
//    data +=
//        _previous_session_id.copy(reinterpret_cast<char *>(data), sid_length);
//  }
//  data = std::copy(_auth.begin(), _auth.end(), data);
//
//  uint32_t size = data - buf.data();
//  std::copy(&size, &size + sizeof(size), buf.data());
//
//  return size;
//}

}  // namespace dsa
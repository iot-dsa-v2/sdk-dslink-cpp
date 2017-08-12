#include "dsa_common.h"

#include "server_connection.h"
#include "core/server.h"

namespace dsa {

ServerConnection::ServerConnection(boost::asio::io_service::strand &strand, const Config &config)
  : Connection(strand, config) {}

void ServerConnection::on_connect() throw(const std::runtime_error &) {
  // setup session now that client session id has been parsed
  _server->session_manager().get_session(
    _other_dsid, _other_token, _session_id, [=](const intrusive_ptr_<Session> &session) {
    if (session != nullptr) {
      _session = session;
      _session_id = _session->session_id();
      _session->set_connection(shared_from_this());
      _session->start();
    } else {
      // TODO, send error
    }
  });

}

///////////////////////
// Handshake Functions
///////////////////////

bool ServerConnection::parse_f0(size_t size) {
  if (size < MinF0Length)
    return false;

  const uint8_t *data = _read_buffer->data();

  StaticHeaders header(data);

  if (!valid_handshake_header(header, size, MessageType::Handshake0))
    return false;

  data += StaticHeaders::TotalSize;
  uint8_t dsid_length;

  _dsa_version_major = *data++;
  _dsa_version_minor = *data++;
  dsid_length = *data++;

  if ((data - _read_buffer->data()) + dsid_length + PublicKeyLength + 1 + SaltLength > size)
    return false;

  data += _other_dsid.assign(reinterpret_cast<const char *>(data), dsid_length).size();
  data += _other_public_key.assign(reinterpret_cast<const char *>(data), PublicKeyLength).size();
  _security_preference = *data++;
  data += _other_salt.assign(reinterpret_cast<const char *>(data), SaltLength).size();

  return data == _read_buffer->data() + size;
}

bool ServerConnection::parse_f2(size_t size) {
  if (size < MinF2Length)
    return false;

  const uint8_t *data = _read_buffer->data();

  StaticHeaders header(data);
  if (!valid_handshake_header(header, size, MessageType::Handshake2))
    return false;

  data += StaticHeaders::TotalSize;
  uint16_t token_length;
  uint16_t session_id_length;

  std::copy(data, data + sizeof(token_length), &token_length);
  data += sizeof(token_length);

  // prevent accidental read in unowned memory
  if ((data - _read_buffer->data()) + token_length + 2 + sizeof(session_id_length) > size)
    return false;

  data += _other_token.assign(reinterpret_cast<const char *>(data), token_length).size();
  _is_requester = (*data++ != 0u);
  _is_responder = (*data++ != 0u);
  std::copy(data, data + sizeof(session_id_length), &session_id_length);
  data += sizeof(session_id_length);

  // prevent accidental read in unowned memory
  if ((data - _read_buffer->data()) + session_id_length + AuthLength != size)
    return false;

  data += _session_id.assign(reinterpret_cast<const char *>(data), session_id_length).size();
  data += _other_auth.assign(reinterpret_cast<const char *>(data), AuthLength).size();

  return data == _read_buffer->data() + size;
}

size_t ServerConnection::load_f1(Buffer &buf) {
  auto dsid_length = (uint8_t) _handshake_context.dsid().size();

  // ensure buf is large enough
  buf.resize(MinF1Length + dsid_length);

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake1, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  data += StaticHeaders::TotalSize;
  (*data++) = dsid_length;
  data += _handshake_context.dsid().copy(reinterpret_cast<char *>(data), dsid_length);
  data += _handshake_context.public_key().copy(reinterpret_cast<char *>(data), PublicKeyLength);
  data += _handshake_context.salt().copy(reinterpret_cast<char *>(data), SaltLength);

  uint32_t size = data - buf.data();
  std::copy(&size, &size + sizeof(size), buf.data());

  return size;
}

size_t ServerConnection::load_f3(Buffer &buf) {
  auto sid_length = (uint16_t) _session_id.size();
  auto path_length = (uint16_t) _path.size();

  // ensure buf is large enough
  buf.resize(MinF2Length + sid_length);

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake3, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  data += StaticHeaders::TotalSize;
  data = std::copy(&sid_length, &sid_length + sizeof(sid_length), data);
  data += _session_id.copy(reinterpret_cast<char *>(data), sid_length);
  data = std::copy(&path_length, &path_length + sizeof(path_length), data);
  data += _path.copy(reinterpret_cast<char *>(data), path_length);
  data += _auth.copy(reinterpret_cast<char *>(data), AuthLength);

  uint32_t size = data - buf.data();
  std::copy(&size, &size + sizeof(size), buf.data());

  return size;
}

}  // namespace dsa
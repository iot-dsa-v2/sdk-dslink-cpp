#include "dsa_common.h"

#include "core/session.h"
#include "client_connection.h"

#define DEBUG 0

namespace dsa {

ClientConnection::ClientConnection(boost::asio::io_service::strand &strand, const Config &config)
  : Connection(strand, config) {}

void ClientConnection::on_connect() throw(const std::runtime_error &) {
  if (_session == nullptr)
    throw std::runtime_error("no session attached to client connection");
  _session->start();
}

///////////////////////
// Handshake Functions
///////////////////////

bool ClientConnection::parse_f1(size_t size) {
  if (size < MinF1Length)
    return false;

  const uint8_t *data = _read_buffer->data();

  StaticHeaders header(data);
  if (!valid_handshake_header(header, size, MessageType::Handshake1))
    return false;

  data += StaticHeaders::TotalSize;
  uint8_t dsid_length;

  std::copy(data, data + sizeof(dsid_length), &dsid_length);
  data += sizeof(dsid_length);

  if ((data - _read_buffer->data()) + dsid_length + PublicKeyLength + SaltLength > size)
    return false;

  data += _other_dsid.assign(reinterpret_cast<const char *>(data), dsid_length).size();
  data += _other_public_key.assign(reinterpret_cast<const char *>(data), PublicKeyLength).size();
  data += _other_salt.assign(reinterpret_cast<const char *>(data), SaltLength).size();

  return data == _read_buffer->data() + size;
}

bool ClientConnection::parse_f3(size_t size) {
  if (size < MinF3Length)
    return false;

  const uint8_t *data = _read_buffer->data();

  StaticHeaders header(data);
  if (!valid_handshake_header(header, size, MessageType::Handshake3))
    return false;

  uint32_t cur = StaticHeaders::TotalSize;
  uint16_t session_id_length, path_length;

  std::copy(data, data + sizeof(session_id_length), &session_id_length);
  data += sizeof(session_id_length);
  if (cur + session_id_length + sizeof(path_length) > size)
    return false;
  data += _session_id.assign(reinterpret_cast<const char *>(data), session_id_length).size();

  std::copy(data, data + sizeof(path_length), &path_length);
  data += sizeof(path_length);
  if (cur + path_length + AuthLength > size)
    return false;
  data += _path.assign(reinterpret_cast<const char *>(data), path_length).size();

  data += _auth_check.assign(reinterpret_cast<const char *>(data), AuthLength).size();

#if DEBUG
  std::stringstream ss;
  ss << name() << "::client_parse_f3()" << std::endl;
  ss << "auth check: " << *auth_check << std::endl;
  ss << "other auth: " << *_other_auth << std::endl;
  std::cout << ss.str();
#endif

  // make sure other auth is correct
  if (_auth_check != _other_auth)
    return false;

  return data == _read_buffer->data() + size;
}

// Handshake load functions
size_t ClientConnection::load_f0(Buffer &buf) {
  uint8_t dsid_length = (uint8_t) _handshake_context.dsid().size();

  // ensure buf is large enough
  buf.resize(MinF0Length + _handshake_context.dsid().size());

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake0, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  data += StaticHeaders::TotalSize;
  (*data++) = (uint8_t) 2; // version major
  (*data++) = (uint8_t) 0; // version major
  (*data++) = dsid_length;
  data += _handshake_context.dsid().copy(reinterpret_cast<char*>(data), dsid_length);
  data += _handshake_context.public_key().copy(reinterpret_cast<char*>(data), PublicKeyLength);
  (*data++) = (uint8_t) 0; // no encryption for now
  data += _handshake_context.salt().copy(reinterpret_cast<char*>(data), SaltLength);

  uint32_t size = data - buf.data();
  std::copy(&size, &size + sizeof(size), buf.data());

  return size;
}

size_t ClientConnection::load_f2(Buffer &buf) {
  auto token_length = (uint16_t) _config.client_token.size();
  auto sid_length = (uint16_t)_previous_session_id.size();

  // ensure buf is large enough
  buf.resize(MinF2Length + token_length);

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake2, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  data += StaticHeaders::TotalSize;
  data = std::copy(&token_length, &token_length + sizeof(token_length), data);
  data += _config.client_token.copy(reinterpret_cast<char*>(data), token_length);
  (*data++) = (uint8_t) (_is_requester ? 1 : 0);
  (*data++) = (uint8_t) (_is_responder ? 1 : 0);
  data = std::copy(&sid_length, &sid_length + sizeof(sid_length), data);
  if (sid_length > 0) {
    data += _previous_session_id.copy(reinterpret_cast<char*>(data), sid_length);
  }
  data += _auth.copy(reinterpret_cast<char*>(data), AuthLength);

  uint32_t size = data - buf.data();
  std::copy(&size, &size + sizeof(size), buf.data());

  return size;
}


}  // namespace dsa
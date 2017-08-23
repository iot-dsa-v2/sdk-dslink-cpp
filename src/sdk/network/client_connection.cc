#include "dsa_common.h"

#include "connection.h"
#include "core/client.h"

#define DEBUG 0

namespace dsa {



void Connection::on_client_connect() throw(const std::runtime_error &) {
  if (_session == nullptr)
    throw std::runtime_error("no session attached to client connection");
  _session->start();
}

///////////////////////
// Handshake Functions
///////////////////////

bool Connection::parse_f1(size_t size) {
  if (size < MinF1Length) return false;

  const uint8_t *data = _write_buffer->data();

  StaticHeaders header(data);
  if (!valid_handshake_header(header, size, MessageType::Handshake1))
    return false;

  data += StaticHeaders::TotalSize;
  uint8_t dsid_length;

  std::copy(data, data + sizeof(dsid_length), &dsid_length);
  data += sizeof(dsid_length);

  if ((data - _write_buffer->data()) + dsid_length + PublicKeyLength +
          SaltLength >
      size)
    return false;

  data += _other_dsid.assign(reinterpret_cast<const char *>(data), dsid_length)
              .size();

  _other_public_key.assign(data, data + PublicKeyLength);
  data += _other_public_key.size();

  _other_salt.assign(data, data + SaltLength);
  data += _other_salt.size();

  return data == _write_buffer->data() + size;
}

bool Connection::parse_f3(size_t size) {
  if (size < MinF3Length) return false;

  const uint8_t *data = _write_buffer->data();

  StaticHeaders header(data);
  if (!valid_handshake_header(header, size, MessageType::Handshake3))
    return false;

  uint32_t cur = StaticHeaders::TotalSize;
  uint16_t session_id_length, path_length;

  std::copy(data, data + sizeof(session_id_length), &session_id_length);
  data += sizeof(session_id_length);
  if (cur + session_id_length + sizeof(path_length) > size) return false;
  data += _session_id
              .assign(reinterpret_cast<const char *>(data), session_id_length)
              .size();

  std::copy(data, data + sizeof(path_length), &path_length);
  data += sizeof(path_length);
  if (cur + path_length + AuthLength > size) return false;
  data +=
      _path.assign(reinterpret_cast<const char *>(data), path_length).size();

  _auth_check.assign(data, data + AuthLength);
  data += _auth_check.size();

#if DEBUG
  std::stringstream ss;
  ss << name() << "::client_parse_f3()" << std::endl;
  ss << "auth check: " << *auth_check << std::endl;
  ss << "other auth: " << *_other_auth << std::endl;
  std::cout << ss.str();
#endif

  // make sure other auth is correct
  if (_auth_check != _other_auth) return false;

  return data == _write_buffer->data() + size;
}

// Handshake load functions
size_t Connection::load_f0(ByteBuffer &buf) {
  uint16_t dsid_length =
      static_cast<uint16_t>(_handshake_context.dsid().size());

  // ensure buf is large enough
  buf.resize(MinF0Length + _handshake_context.dsid().size());

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake0, 0,
                       0);
  uint8_t *data = buf.data();
  header.write(data);
  data += StaticHeaders::TotalSize;
  (*data++) = (uint8_t)2;  // version major
  (*data++) = (uint8_t)0;  // version major
  data = std::copy(&dsid_length, &dsid_length + sizeof(dsid_length), data);
  data += _handshake_context.dsid().copy(reinterpret_cast<char *>(data),
                                         dsid_length);
  data = std::copy(_handshake_context.public_key().begin(),
                   _handshake_context.public_key().end(), data);
  (*data++) = (uint8_t)0;  // no encryption for now
  // TODO: implement encryption
  data = std::copy(_handshake_context.salt().begin(),
                   _handshake_context.salt().end(), data);

  uint32_t size = data - buf.data();
  std::copy(&size, &size + sizeof(size), buf.data());

  return size;
}

size_t Connection::load_f2(ByteBuffer &buf) {
  auto token_length = (uint16_t)_client_token.size();
  auto sid_length = (uint16_t)_previous_session_id.size();

  // ensure buf is large enough
  buf.resize(MinF2Length + token_length);

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake2, 0,
                       0);
  uint8_t *data = buf.data();
  header.write(data);
  data += StaticHeaders::TotalSize;
  data = std::copy(&token_length, &token_length + sizeof(token_length), data);
  data += _client_token.copy(reinterpret_cast<char *>(data), token_length);
  (*data++) = (uint8_t)(_is_requester ? 1 : 0);
  (*data++) = (uint8_t)(_is_responder ? 1 : 0);
  data = std::copy(&sid_length, &sid_length + sizeof(sid_length), data);
  if (sid_length > 0) {
    data +=
        _previous_session_id.copy(reinterpret_cast<char *>(data), sid_length);
  }
  data = std::copy(_auth.begin(), _auth.end(), data);

  uint32_t size = data - buf.data();
  std::copy(&size, &size + sizeof(size), buf.data());

  return size;
}

}  // namespace dsa
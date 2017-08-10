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
  if (!valid_handshake_header(header, size, 0xf1))
    return false;

  uint32_t cur = StaticHeaders::TotalSize;
  uint8_t dsid_length;

  std::memcpy(&dsid_length, &data[cur], sizeof(dsid_length));
  cur += sizeof(dsid_length);

  if (cur + dsid_length + PublicKeyLength + SaltLength > size)
    return false;

  _other_dsid = make_intrusive_<Buffer>(dsid_length);
  _other_dsid->assign(&data[cur], dsid_length);
  cur += dsid_length;
  _other_public_key = make_intrusive_<Buffer>(PublicKeyLength);
  _other_public_key->assign(&data[cur], PublicKeyLength);
  cur += PublicKeyLength;
  _other_salt = make_intrusive_<Buffer>(SaltLength);
  _other_salt->assign(&data[cur], SaltLength);
  cur += SaltLength;

  return cur == size;
}

bool ClientConnection::parse_f3(size_t size) {
  if (size < MinF3Length)
    return false;

  const uint8_t *data = _read_buffer->data();

  StaticHeaders header(data);
  if (!valid_handshake_header(header, size, 0xf3))
    return false;

  uint32_t cur = StaticHeaders::TotalSize;
  uint16_t session_id_length, path_length;

  std::memcpy(&session_id_length, &data[cur], sizeof(session_id_length));
  cur += sizeof(session_id_length);

  if (cur + session_id_length + sizeof(path_length) > size)
    return false;

  _session_id = make_intrusive_<Buffer>(session_id_length);
  _session_id->assign(&data[cur], session_id_length);
  cur += session_id_length;
  std::memcpy(&path_length, &data[cur], sizeof(path_length));
  cur += sizeof(path_length);

  if (cur + path_length + AuthLength > size)
    return false;

  _path = make_intrusive_<Buffer>(path_length);
  _path->assign(&data[cur], path_length);
  cur += path_length;
  auto auth_check = make_intrusive_<Buffer>(AuthLength);
  auth_check->assign(&data[cur], AuthLength);

#if DEBUG
  std::stringstream ss;
  ss << name() << "::client_parse_f3()" << std::endl;
  ss << "auth check: " << *auth_check << std::endl;
  ss << "other auth: " << *_other_auth << std::endl;
  std::cout << ss.str();
#endif

  // make sure other auth is correct
  if (*auth_check != *_other_auth)
    return false;

  cur += AuthLength;

  return cur == size;
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
  uint32_t cur = StaticHeaders::TotalSize;
  data[cur] = (uint8_t) 2; // version major
  data[++cur] = (uint8_t) 0; // version minor
  data[++cur] = dsid_length;
  std::memcpy(&data[++cur], _handshake_context.dsid().c_str(), dsid_length);
  cur += dsid_length;
  std::memcpy(&data[cur], _handshake_context.public_key().data(), PublicKeyLength);
  cur += PublicKeyLength;
  data[cur++] = 0; // no encryption for now
  std::memcpy(&data[cur], _handshake_context.salt().data(), SaltLength);
  cur += SaltLength;
  std::memcpy(data, &cur, sizeof(cur)); // write total size

  return cur;
}

size_t ClientConnection::load_f2(Buffer &buf) {
  auto token_length = (uint16_t) _config.client_token.size();
  auto session_id_length = (uint16_t)previous_session.size();

  // ensure buf is large enough
  buf.resize(MinF2Length + token_length);

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake2, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = StaticHeaders::TotalSize;
  std::memcpy(&data[cur], &token_length, sizeof(token_length));
  cur += sizeof(token_length);
  std::memcpy(&data[cur], &_config.client_token[0], token_length);
  cur += token_length;
  data[cur++] = (uint8_t) (_is_requester ? 1 : 0);
  data[cur++] = (uint8_t) (_is_responder ? 1 : 0);
  std::memcpy(&data[cur], &session_id_length, sizeof(session_id_length));
  cur += sizeof(session_id_length);
  if (session_id_length > 0) {
    std::memcpy(&data[cur], &previous_session[0], session_id_length);
    cur += session_id_length;
  }
  std::memcpy(&data[cur], _auth->data(), AuthLength);
  cur += AuthLength;
  std::memcpy(data, &cur, sizeof(cur));

  return cur;
}


}  // namespace dsa
#include "connection.h"

#include "security_context.h"

namespace dsa {

Connection::Connection(const App &app) : _app(app), _read_buffer(new Buffer()), _write_buffer(new Buffer()) {}

void Connection::set_read_handler(ReadCallback callback) {
  read_handler = callback;
}

void Connection::handle_read(Buffer::MessageBuffer buf) {
  read_handler(buf);
}

void Connection::success_or_close(const boost::system::error_code &error) {
  if (error) close();
}

void Connection::destroy() {
//  EnableShared<Connection>::destroy();
  close();
}

void Connection::compute_secret() {
  _shared_secret = _app.security_context().ecdh().compute_secret(*_other_public_key);

  /* compute user auth */
  dsa::HMAC hmac("sha256", *_shared_secret);
  hmac.update(*_other_salt);
  _auth = hmac.digest();

  /* compute other auth */
  dsa::HMAC other_hmac("sha256", *_shared_secret);
  other_hmac.update(_app.security_context().salt());
  _other_auth = other_hmac.digest();
}

bool Connection::valid_handshake_header(StaticHeader &header, size_t expected_size, uint8_t expected_type) {
  return (
      header.message_size() != expected_size &&
      header.header_size() != STATIC_HEADER_LENGTH &&
      header.type() != expected_type &&
      header.request_id() != 0 &&
      header.ack_id() != 0
  );
}

// Handshake parse functions
bool Connection::parse_f0(size_t size) {
  if (size < MIN_F0_LENGTH)
    return false;

  const uint8_t *data = _read_buffer->data();

  StaticHeader header(data);
  if (!valid_handshake_header(header, size, 0xf0))
    return false;

  uint32_t cur = STATIC_HEADER_LENGTH;
  uint8_t dsid_length;

  std::memcpy(&_dsa_version_major, &data[cur], sizeof(_dsa_version_major));
  cur += sizeof(_dsa_version_major);
  std::memcpy(&_dsa_version_minor, &data[cur], sizeof(_dsa_version_minor));
  cur += sizeof(_dsa_version_minor);
  std::memcpy(&dsid_length, &data[cur], sizeof(dsid_length));
  cur += sizeof(dsid_length);

  if (cur + dsid_length + PUBLIC_KEY_LENGTH + 1 + SALT_LENGTH > size) return false;

  _other_dsid = std::make_shared<Buffer>(dsid_length);
  _other_dsid->assign(&data[cur], dsid_length);

  cur += dsid_length;
  _other_public_key = std::make_shared<Buffer>(PUBLIC_KEY_LENGTH);
  _other_public_key->assign(&data[cur], PUBLIC_KEY_LENGTH);
  cur += PUBLIC_KEY_LENGTH;
  _security_preference = data[cur];
  cur += 1;
  _other_salt = std::make_shared<Buffer>(SALT_LENGTH);
  _other_salt->assign(&data[cur], SALT_LENGTH);

  return cur == size;
}

bool Connection::parse_f1(size_t size) {
  if (size < MIN_F1_LENGTH)
    return false;

  const uint8_t *data = _read_buffer->data();

  StaticHeader header(data);
  if (!valid_handshake_header(header, size, 0xf1))
    return false;

  uint32_t cur = STATIC_HEADER_LENGTH;
  uint8_t dsid_length;

  std::memcpy(&dsid_length, &data[cur], sizeof(dsid_length));
  cur += sizeof(dsid_length);

  if (cur + dsid_length + PUBLIC_KEY_LENGTH + SALT_LENGTH > size)
    return false;

  _other_dsid = std::make_shared<Buffer>(dsid_length);
  _other_dsid->assign(&data[cur], dsid_length);
  cur += dsid_length;
  _other_public_key = std::make_shared<Buffer>(PUBLIC_KEY_LENGTH);
  _other_public_key->assign(&data[cur], PUBLIC_KEY_LENGTH);
  cur += PUBLIC_KEY_LENGTH;
  _other_salt = std::make_shared<Buffer>(SALT_LENGTH);
  _other_salt->assign(&data[cur], SALT_LENGTH);

  return cur == size;
}

bool Connection::parse_f2(size_t size) {
  if (size < MIN_F2_LENGTH)
    return false;

  const uint8_t *data = _read_buffer->data();

  StaticHeader header(data);
  if (!valid_handshake_header(header, size, 0xf2))
    return false;

  uint32_t cur = STATIC_HEADER_LENGTH;
  uint16_t token_length;

  std::memcpy(&token_length, &data[cur], sizeof(token_length));
  cur += sizeof(token_length);

  if (cur + token_length + 2 + AUTH_LENGTH > size)
    return false;

  _other_token = std::make_shared<Buffer>(token_length);
  _other_token->assign(&data[cur], token_length);
  cur += token_length;
  _is_requester = data[cur++];
  _is_responder = data[cur++];
  _other_auth = std::make_shared<Buffer>(AUTH_LENGTH);
  _other_auth->assign(&data[cur], AUTH_LENGTH);

  return cur == size;
}

bool Connection::parse_f3(size_t size) {
  if (size < MIN_F3_LENGTH)
    return false;

  const uint8_t *data = _read_buffer->data();

  StaticHeader header(data);
  if (!valid_handshake_header(header, size, 0xf3))
    return false;

  uint32_t cur = STATIC_HEADER_LENGTH;
  uint16_t session_id_length, path_length;

  std::memcpy(&session_id_length, &data[cur], sizeof(session_id_length));
  cur += sizeof(session_id_length);

  if (cur + session_id_length + sizeof(path_length) > size)
    return false;

  _session_id = std::make_shared<Buffer>(session_id_length);
  _session_id->assign(&data[cur], session_id_length);
  cur += session_id_length;
  std::memcpy(&path_length, &data[cur], sizeof(path_length));
  cur += sizeof(path_length);

  if (cur + path_length + AUTH_LENGTH > size)
    return false;

  _path = std::make_shared<Buffer>(path_length);
  _path->assign(&data[cur], path_length);
  cur += path_length;
  _other_auth = std::make_shared<Buffer>(AUTH_LENGTH);
  _other_auth->assign(&data[cur], AUTH_LENGTH);
  cur += AUTH_LENGTH;

  return cur == size;
}

// Handshake load functions
size_t Connection::load_f0(Buffer &buf) {
  uint8_t dsid_length = (uint8_t)_app.security_context().dsid().size();

  // ensure buf is large enough
  buf.resize(MIN_F0_LENGTH + _app.security_context().dsid().size());

  // leave message size blank for now
  StaticHeader header(0, STATIC_HEADER_LENGTH, 0xf0, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = STATIC_HEADER_LENGTH;
  data[cur++] = 2; // version major
  data[cur++] = 0; // version minor
  data[cur++] = dsid_length;
  std::memcpy(&data[cur], _app.security_context().dsid().c_str(), dsid_length);
  cur += dsid_length;
  std::memcpy(&data[cur], _app.security_context().public_key().data(), PUBLIC_KEY_LENGTH);
  data[cur++] = 0; // no encryption for now
  std::memcpy(&data[cur], _app.security_context().salt().data(), SALT_LENGTH);
  cur += SALT_LENGTH;
  std::memcpy(data, &cur, sizeof(cur)); // write total size

  return cur;
}

size_t Connection::load_f1(Buffer &buf) {
  uint8_t dsid_length = (uint8_t)_app.security_context().dsid().size();

  // ensure buf is large enough
  buf.resize(MIN_F1_LENGTH + dsid_length);

  // leave message size blank for now
  StaticHeader header(0, STATIC_HEADER_LENGTH, 0xf1, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = STATIC_HEADER_LENGTH;
  data[cur++] = dsid_length;
  std::memcpy(&data[cur], _app.security_context().dsid().c_str(), dsid_length);
  cur += dsid_length;
  std::memcpy(&data[cur], _app.security_context().public_key().data(), PUBLIC_KEY_LENGTH);
  cur += PUBLIC_KEY_LENGTH;
  std::memcpy(&data[cur], _app.security_context().salt().data(), SALT_LENGTH);
  cur += SALT_LENGTH;
  std::memcpy(data, &cur, sizeof(cur));

  return cur;
}

size_t Connection::load_f2(Buffer &buf) {
  uint16_t token_length = (uint16_t)_token->size();

  // ensure buf is large enough
  buf.resize(MIN_F2_LENGTH + token_length);

  // leave message size blank for now
  StaticHeader header(0, STATIC_HEADER_LENGTH, 0xf2, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = STATIC_HEADER_LENGTH;
  std::memcpy(&data[cur], &token_length, sizeof(token_length));
  cur += sizeof(token_length);
  std::memcpy(&data[cur], _token->data(), token_length);
  cur += token_length;
  data[cur++] = (uint8_t)(_is_requester ? 1 : 0);
  data[cur++] = (uint8_t)(_is_responder ? 1 : 0);
  std::memcpy(&data[cur], _auth->data(), AUTH_LENGTH);
  cur += AUTH_LENGTH;
  std::memcpy(data, &cur, sizeof(cur));

  return cur;
}

size_t Connection::load_f3(Buffer &buf) {
  uint16_t session_id_length = (uint16_t)_session_id->size();
  uint16_t path_length = (uint16_t)_path->size();

  // ensure buf is large enough
  buf.resize(MIN_F2_LENGTH + session_id_length);

  // leave message size blank for now
  StaticHeader header(0, STATIC_HEADER_LENGTH, 0xf3, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = STATIC_HEADER_LENGTH;
  std::memcpy(&data[cur], &session_id_length, sizeof(session_id_length));
  cur += sizeof(session_id_length);
  std::memcpy(&data[cur], _session_id->data(), session_id_length);
  cur += session_id_length;
  std::memcpy(&data[cur], &path_length, sizeof(path_length));
  cur += sizeof(path_length);
  std::memcpy(&data[cur], _path->data(), path_length);
  cur += path_length;
  std::memcpy(&data[cur], _auth->data(), AUTH_LENGTH);
  cur += AUTH_LENGTH;
  std::memcpy(data, &cur, sizeof(cur));

  return cur;
}

}  // namespace dsa
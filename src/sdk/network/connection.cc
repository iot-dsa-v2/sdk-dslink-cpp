#include "connection.h"

#include <boost/asio.hpp>
#include <utility>

#include "app.h"

namespace dsa {

Connection::Connection(const App &app, const Config &config)
    : _app(app), _read_buffer(new Buffer()), _write_buffer(new Buffer()), _config(config) {}

void Connection::set_read_handler(ReadHandler handler) {
  _read_handler.reset(new ReadHandler(handler));
}

void Connection::handle_read(Buffer::SharedBuffer buf) {
  (*_read_handler)(std::move(buf));
}

void Connection::success_or_close(const boost::system::error_code &error) {
  if (error != nullptr) close();
}

//void Connection::destroy() {
////  EnableShared<Connection>::destroy();
//  close();
//}

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

bool Connection::valid_handshake_header(StaticHeaders &header, size_t expected_size, uint8_t expected_type) {
  return (
      header.message_size() == expected_size &&
          header.header_size() == StaticHeaders::TotalSize &&
          header.type() == expected_type &&
          header.request_id() == 0 &&
          header.ack_id() == 0
  );
}

// Handshake parse functions
bool Connection::parse_f0(size_t size) {
  if (size < MinF0Length)
    return false;

  const uint8_t *data = _read_buffer->data();

  StaticHeaders header(data);

  if (!valid_handshake_header(header, size, 0xf0))
    return false;

  uint32_t cur = StaticHeaders::TotalSize;
  uint8_t dsid_length;

  std::memcpy(&_dsa_version_major, &data[cur], sizeof(_dsa_version_major));
  cur += sizeof(_dsa_version_major);
  std::memcpy(&_dsa_version_minor, &data[cur], sizeof(_dsa_version_minor));
  cur += sizeof(_dsa_version_minor);
  std::memcpy(&dsid_length, &data[cur], sizeof(dsid_length));
  cur += sizeof(dsid_length);

  if (cur + dsid_length + PublicKeyLength + 1 + SaltLength > size) return false;

  _other_dsid = std::make_shared<Buffer>(dsid_length);
  _other_dsid->assign(&data[cur], dsid_length);

  cur += dsid_length;
  _other_public_key = std::make_shared<Buffer>(PublicKeyLength);
  _other_public_key->assign(&data[cur], PublicKeyLength);
  cur += PublicKeyLength;
  _security_preference = data[cur++];
  _other_salt = std::make_shared<Buffer>(SaltLength);
  _other_salt->assign(&data[cur], SaltLength);
  cur += SaltLength;

  return cur == size;
}

bool Connection::parse_f0(Buffer &buf, size_t size) {
  if (size < MinF0Length)
    return false;

  const uint8_t *data = buf.data();

  StaticHeaders header(data);

  if (!valid_handshake_header(header, size, 0xf0))
    return false;

  uint32_t cur = StaticHeaders::TotalSize;
  uint8_t dsid_length;

  std::memcpy(&_dsa_version_major, &data[cur], sizeof(_dsa_version_major));
  cur += sizeof(_dsa_version_major);
  std::memcpy(&_dsa_version_minor, &data[cur], sizeof(_dsa_version_minor));
  cur += sizeof(_dsa_version_minor);
  std::memcpy(&dsid_length, &data[cur], sizeof(dsid_length));
  cur += sizeof(dsid_length);

  if (cur + dsid_length + PublicKeyLength + 1 + SaltLength > size) return false;

  _other_dsid = std::make_shared<Buffer>(dsid_length);
  _other_dsid->assign(&data[cur], dsid_length);

  cur += dsid_length;
  _other_public_key = std::make_shared<Buffer>(PublicKeyLength);
  _other_public_key->assign(&data[cur], PublicKeyLength);
  cur += PublicKeyLength;
  _security_preference = (data[cur++] != 0u);
  _other_salt = std::make_shared<Buffer>(SaltLength);
  _other_salt->assign(&data[cur], SaltLength);
  cur += SaltLength;

  return cur == size;
}

bool Connection::parse_f1(size_t size) {
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

  _other_dsid = std::make_shared<Buffer>(dsid_length);
  _other_dsid->assign(&data[cur], dsid_length);
  cur += dsid_length;
  _other_public_key = std::make_shared<Buffer>(PublicKeyLength);
  _other_public_key->assign(&data[cur], PublicKeyLength);
  cur += PublicKeyLength;
  _other_salt = std::make_shared<Buffer>(SaltLength);
  _other_salt->assign(&data[cur], SaltLength);
  cur += SaltLength;

  return cur == size;
}

bool Connection::parse_f2(size_t size) {
  if (size < MinF2Length)
    return false;

  const uint8_t *data = _read_buffer->data();

  StaticHeaders header(data);
  if (!valid_handshake_header(header, size, 0xf2))
    return false;

  uint32_t cur = StaticHeaders::TotalSize;
  uint16_t token_length;
  uint16_t session_id_length;

  std::memcpy(&token_length, &data[cur], sizeof(token_length));
  cur += sizeof(token_length);

  // prevent accidental read in unowned memory
  if (cur + token_length + 2 + sizeof(session_id_length) > size)
    return false;

  _other_token = std::make_shared<Buffer>(token_length);
  _other_token->assign(&data[cur], token_length);
  cur += token_length;

  _is_requester = (data[cur++] != 0u);
  _is_responder = (data[cur++] != 0u);
  std::memcpy(&session_id_length, &data[cur], sizeof(session_id_length));
  cur += sizeof(session_id_length);

  // prevent accidental read in unowned memory
  if (cur + session_id_length + AuthLength != size)
    return false;

  _session_id = std::make_shared<Buffer>(session_id_length);
  _session_id->assign(&data[cur], session_id_length);
  cur += session_id_length;
  _other_auth = std::make_shared<Buffer>(AuthLength);
  _other_auth->assign(&data[cur], AuthLength);
  cur += AuthLength;

  return cur == size;
}

bool Connection::parse_f3(size_t size) {
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

  _session_id = std::make_shared<Buffer>(session_id_length);
  _session_id->assign(&data[cur], session_id_length);
  cur += session_id_length;
  std::memcpy(&path_length, &data[cur], sizeof(path_length));
  cur += sizeof(path_length);

  if (cur + path_length + AuthLength > size)
    return false;

  _path = std::make_shared<Buffer>(path_length);
  _path->assign(&data[cur], path_length);
  cur += path_length;
  _other_auth = std::make_shared<Buffer>(AuthLength);
  _other_auth->assign(&data[cur], AuthLength);
  cur += AuthLength;

  return cur == size;
}

// Handshake load functions
size_t Connection::load_f0(Buffer &buf) {
  uint8_t dsid_length = (uint8_t) _app.security_context().dsid().size();

  // ensure buf is large enough
  buf.resize(MinF0Length + _app.security_context().dsid().size());

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, 0xf0, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = StaticHeaders::TotalSize;
  data[cur] = (uint8_t) 2; // version major
  data[++cur] = (uint8_t) 0; // version minor
  data[++cur] = dsid_length;
  std::memcpy(&data[++cur], _app.security_context().dsid().c_str(), dsid_length);
  cur += dsid_length;
  std::memcpy(&data[cur], _app.security_context().public_key().data(), PublicKeyLength);
  cur += PublicKeyLength;
  data[cur++] = 0; // no encryption for now
  std::memcpy(&data[cur], _app.security_context().salt().data(), SaltLength);
  cur += SaltLength;
  std::memcpy(data, &cur, sizeof(cur)); // write total size

//  parse_f0(buf, cur);
//  std::cout << "f0 valid: " << parse_f0(buf, cur) << std::endl;

  return cur;
}

size_t Connection::load_f1(Buffer &buf) {
  auto dsid_length = (uint8_t) _app.security_context().dsid().size();

  // ensure buf is large enough
  buf.resize(MinF1Length + dsid_length);

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, 0xf1, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = StaticHeaders::TotalSize;
  data[cur++] = dsid_length;
  std::memcpy(&data[cur], _app.security_context().dsid().c_str(), dsid_length);
  cur += dsid_length;
  std::memcpy(&data[cur], _app.security_context().public_key().data(), PublicKeyLength);
  cur += PublicKeyLength;
  std::memcpy(&data[cur], _app.security_context().salt().data(), SaltLength);
  cur += SaltLength;
  std::memcpy(data, &cur, sizeof(cur));

  return cur;
}

size_t Connection::load_f2(Buffer &buf) {
  auto token_length = (uint16_t)_config.token()->size();
  auto session_id_length = (uint16_t)_config.session_id()->size();

  // ensure buf is large enough
  buf.resize(MinF2Length + token_length);

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, 0xf2, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = StaticHeaders::TotalSize;
  std::memcpy(&data[cur], &token_length, sizeof(token_length));
  cur += sizeof(token_length);
  std::memcpy(&data[cur], _config.token()->data(), token_length);
  cur += token_length;
  data[cur++] = (uint8_t) (_is_requester ? 1 : 0);
  data[cur++] = (uint8_t) (_is_responder ? 1 : 0);
  std::memcpy(&data[cur], &session_id_length, sizeof(session_id_length));
  cur += sizeof(session_id_length);
  if (session_id_length > 0) {
    std::memcpy(&data[cur], _config.session_id()->data(), session_id_length);
    cur += session_id_length;
  }
  std::memcpy(&data[cur], _auth->data(), AuthLength);
  cur += AuthLength;
  std::memcpy(data, &cur, sizeof(cur));

  return cur;
}

size_t Connection::load_f3(Buffer &buf) {
  auto session_id_length = (uint16_t) _session_id->size();
  auto path_length = (uint16_t) _path->size();

  // ensure buf is large enough
  buf.resize(MinF2Length + session_id_length);

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, 0xf3, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = StaticHeaders::TotalSize;
  std::memcpy(&data[cur], &session_id_length, sizeof(session_id_length));
  cur += sizeof(session_id_length);
  std::memcpy(&data[cur], _session_id->data(), session_id_length);
  cur += session_id_length;
  std::memcpy(&data[cur], &path_length, sizeof(path_length));
  cur += sizeof(path_length);
  std::memcpy(&data[cur], _path->data(), path_length);
  cur += path_length;
  std::memcpy(&data[cur], _auth->data(), AuthLength);
  cur += AuthLength;
  std::memcpy(data, &cur, sizeof(cur));

  return cur;
}

}  // namespace dsa
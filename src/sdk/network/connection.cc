#include "dsa_common.h"

#include "connection.h"
#include "core/session.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "core/app.h"

namespace dsa {

Connection::Connection(const App &app, const Config &config)
    : _security_context(app.security_context()),
      _read_buffer(new Buffer()),
      _write_buffer(new Buffer()),
      _config(config),
      _deadline(app.io_service()),
      _global_strand(app.strand()),
      _message_handler(config.message_handler()) {}

void Connection::success_or_close(const boost::system::error_code &error) {
  if (error != nullptr) close();
}

void Connection::compute_secret() {
  _shared_secret = _security_context.ecdh().compute_secret(*_other_public_key);

  /* compute user auth */
  dsa::HMAC hmac("sha256", *_shared_secret);
  hmac.update(*_other_salt);
  _auth = hmac.digest();

  /* compute other auth */
  dsa::HMAC other_hmac("sha256", *_shared_secret);
  other_hmac.update(_security_context.salt());
  _other_auth = other_hmac.digest();
}

bool Connection::valid_handshake_header(StaticHeaders &header, size_t expected_size, uint8_t expected_type) {
  return (
      header.message_size == expected_size &&
          header.header_size == StaticHeaders::TotalSize &&
          header.type == expected_type &&
          header.request_id == 0 &&
          header.ack_id == 0
  );
}

void Connection::timeout(const boost::system::error_code &error) {
  if (error != boost::asio::error::operation_aborted) {
    close();
  }
}

void Connection::reset_standard_deadline_timer() {
  _deadline.expires_from_now(boost::posix_time::minutes(1));
  _deadline.async_wait(boost::bind(&Connection::timeout, shared_from_this(), boost::asio::placeholders::error));
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

  _other_dsid = make_intrusive_<Buffer>(dsid_length);
  _other_dsid->assign(&data[cur], dsid_length);

  cur += dsid_length;
  _other_public_key = make_intrusive_<Buffer>(PublicKeyLength);
  _other_public_key->assign(&data[cur], PublicKeyLength);
  cur += PublicKeyLength;
  _security_preference = data[cur++];
  _other_salt = make_intrusive_<Buffer>(SaltLength);
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

  _other_token = make_intrusive_<Buffer>(token_length);
  _other_token->assign(&data[cur], token_length);
  cur += token_length;

  _is_requester = (data[cur++] != 0u);
  _is_responder = (data[cur++] != 0u);
  std::memcpy(&session_id_length, &data[cur], sizeof(session_id_length));
  cur += sizeof(session_id_length);

  // prevent accidental read in unowned memory
  if (cur + session_id_length + AuthLength != size)
    return false;

  _session_id = make_intrusive_<Buffer>(session_id_length);
  _session_id->assign(&data[cur], session_id_length);
  cur += session_id_length;
  _other_auth = make_intrusive_<Buffer>(AuthLength);
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
  _other_auth = make_intrusive_<Buffer>(AuthLength);
  _other_auth->assign(&data[cur], AuthLength);
  cur += AuthLength;

  return cur == size;
}

// Handshake load functions
size_t Connection::load_f0(Buffer &buf) {
  uint8_t dsid_length = (uint8_t) _security_context.dsid().size();

  // ensure buf is large enough
  buf.resize(MinF0Length + _security_context.dsid().size());

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake0, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = StaticHeaders::TotalSize;
  data[cur] = (uint8_t) 2; // version major
  data[++cur] = (uint8_t) 0; // version minor
  data[++cur] = dsid_length;
  std::memcpy(&data[++cur], _security_context.dsid().c_str(), dsid_length);
  cur += dsid_length;
  std::memcpy(&data[cur], _security_context.public_key().data(), PublicKeyLength);
  cur += PublicKeyLength;
  data[cur++] = 0; // no encryption for now
  std::memcpy(&data[cur], _security_context.salt().data(), SaltLength);
  cur += SaltLength;
  std::memcpy(data, &cur, sizeof(cur)); // write total size

  return cur;
}

size_t Connection::load_f1(Buffer &buf) {
  auto dsid_length = (uint8_t) _security_context.dsid().size();

  // ensure buf is large enough
  buf.resize(MinF1Length + dsid_length);

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake1, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = StaticHeaders::TotalSize;
  data[cur++] = dsid_length;
  std::memcpy(&data[cur], _security_context.dsid().c_str(), dsid_length);
  cur += dsid_length;
  std::memcpy(&data[cur], _security_context.public_key().data(), PublicKeyLength);
  cur += PublicKeyLength;
  std::memcpy(&data[cur], _security_context.salt().data(), SaltLength);
  cur += SaltLength;
  std::memcpy(data, &cur, sizeof(cur));

  return cur;
}

size_t Connection::load_f2(Buffer &buf) {
  auto token_length = (uint16_t) _config.token()->size();
  auto session_id_length = (uint16_t) _config.session_id()->size();

  // ensure buf is large enough
  buf.resize(MinF2Length + token_length);

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake2, 0, 0);
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
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake3, 0, 0);
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
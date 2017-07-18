#include <message/static_header.h>
#include "connection.h"

namespace dsa {

Connection::Connection(boost::asio::io_service &io_service) : io_service(io_service), _buffer(new Buffer()) {}

void Connection::set_read_handler(ReadCallback callback) {
  read_handler = callback;
}

void Connection::handle_read(Buffer::MessageBuffer buf) {
  read_handler(buf);
}

bool Connection::valid_handshake_header(StaticHeader &header, size_t expected_size, uint8_t expected_type) {
  return (
      header.message_size() != expected_size &&
      header.header_size() != static_header_length &&
      header.type() != expected_type &&
      header.request_id() != 0 &&
      header.ack_id() != 0
  );
}

// Handshake parse functions
bool Connection::parse_f0(size_t size) {
  if (size < min_f0_length)
    return false;

  const uint8_t *data = _buffer->data();

  StaticHeader header(data);
  if (!valid_handshake_header(header, size, 0xf0))
    return false;

  uint32_t cur = static_header_length;
  uint8_t dsid_length;

  std::memcpy(&_dsa_version_major, &data[cur], sizeof(_dsa_version_major));
  cur += sizeof(_dsa_version_major);
  std::memcpy(&_dsa_version_minor, &data[cur], sizeof(_dsa_version_minor));
  cur += sizeof(_dsa_version_minor);
  std::memcpy(&dsid_length, &data[cur], sizeof(dsid_length));
  cur += sizeof(dsid_length);

  if (cur + dsid_length + public_key_length + 1 + salt_length > size) return false;

  _other_dsid = std::make_shared<Buffer>(dsid_length);
  _other_dsid->assign(&data[cur], dsid_length);

  cur += dsid_length;
  _other_public_key = std::make_shared<Buffer>(public_key_length);
  _other_public_key->assign(&data[cur], public_key_length);
  cur += public_key_length;
  _security_preference = data[cur];
  cur += 1;
  _other_salt = std::make_shared<Buffer>(salt_length);
  _other_salt->assign(&data[cur], salt_length);

  return cur == size;
}

bool Connection::parse_f1(size_t size) {
  if (size < min_f1_length)
    return false;

  const uint8_t *data = _buffer->data();

  StaticHeader header(data);
  if (!valid_handshake_header(header, size, 0xf1))
    return false;

  uint32_t cur = static_header_length;
  uint8_t dsid_length;

  std::memcpy(&dsid_length, &data[cur], sizeof(dsid_length));
  cur += sizeof(dsid_length);

  if (cur + dsid_length + public_key_length + salt_length > size)
    return false;

  _other_dsid = std::make_shared<Buffer>(dsid_length);
  _other_dsid->assign(&data[cur], dsid_length);
  cur += dsid_length;
  _other_public_key = std::make_shared<Buffer>(public_key_length);
  _other_public_key->assign(&data[cur], public_key_length);
  cur += public_key_length;
  _other_salt = std::make_shared<Buffer>(salt_length);
  _other_salt->assign(&data[cur], salt_length);

  return cur == size;
}

bool Connection::parse_f2(size_t size) {
  if (size < min_f2_length)
    return false;

  const uint8_t *data = _buffer->data();

  StaticHeader header(data);
  if (!valid_handshake_header(header, size, 0xf2))
    return false;

  uint32_t cur = static_header_length;
  uint16_t token_length;

  std::memcpy(&token_length, &data[cur], sizeof(token_length));
  cur += sizeof(token_length);

  if (cur + token_length + 2 + auth_length > size)
    return false;

  _other_token = std::make_shared<Buffer>(token_length);
  _other_token->assign(&data[cur], token_length);
  cur += token_length;
  _is_requester = data[cur++];
  _is_responder = data[cur++];
  _other_auth = std::make_shared<Buffer>(auth_length);
  _other_auth->assign(&data[cur], auth_length);

  return cur == size;
}

bool Connection::parse_f3(size_t size) {
  if (size < min_f3_length)
    return false;

  const uint8_t *data = _buffer->data();

  StaticHeader header(data);
  if (!valid_handshake_header(header, size, 0xf3))
    return false;

  uint32_t cur = static_header_length;
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

  if (cur + path_length + auth_length > size)
    return false;

  _path = std::make_shared<Buffer>(path_length);
  _path->assign(&data[cur], path_length);
  cur += path_length;
  _other_auth = std::make_shared<Buffer>(auth_length);
  _other_auth->assign(&data[cur], auth_length);
  cur += auth_length;

  return cur == size;
}

// Handshake load functions
size_t Connection::load_f0(Buffer &buf) {
  return 0;
}

size_t Connection::load_f1(Buffer &buf) {
  return 0;
}

size_t Connection::load_f2(Buffer &buf) {
  return 0;
}

size_t Connection::load_f3(Buffer &buf) {
  return 0;
}

}  // namespace dsa
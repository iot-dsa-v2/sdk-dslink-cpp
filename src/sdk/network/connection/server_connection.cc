#include "dsa_common.h"

#include "server_connection.h"
#include "core/server.h"

namespace dsa {

ServerConnection::ServerConnection(boost::asio::io_service::strand &strand, const Config &config)
  : Connection(strand, config) {}

void ServerConnection::on_connect() throw(const std::runtime_error &) {
  // setup session now that client session id has been parsed
  std::string session_id = _session_id->to_string();
  _server->session_manager().get_session(
    _other_dsid, _other_token, session_id, [=](intrusive_ptr_<Session> &session) {
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

bool ServerConnection::parse_f2(size_t size) {
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

size_t ServerConnection::load_f1(Buffer &buf) {
  auto dsid_length = (uint8_t) _handshake_context.dsid().size();

  // ensure buf is large enough
  buf.resize(MinF1Length + dsid_length);

  // leave message size blank for now
  StaticHeaders header(0, StaticHeaders::TotalSize, MessageType::Handshake1, 0, 0);
  uint8_t *data = buf.data();
  header.write(data);
  uint32_t cur = StaticHeaders::TotalSize;
  data[cur++] = dsid_length;
  std::memcpy(&data[cur], _handshake_context.dsid().c_str(), dsid_length);
  cur += dsid_length;
  std::memcpy(&data[cur], _handshake_context.public_key().data(), PublicKeyLength);
  cur += PublicKeyLength;
  std::memcpy(&data[cur], _handshake_context.salt().data(), SaltLength);
  cur += SaltLength;
  std::memcpy(data, &cur, sizeof(cur));

  return cur;
}

size_t ServerConnection::load_f3(Buffer &buf) {
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
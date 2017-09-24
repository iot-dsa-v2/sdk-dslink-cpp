#include "dsa_common.h"

#include "client.h"
#include "core/session.h"
#include "network/connection.h"

namespace dsa {
Client::Client(WrapperConfig& config)
    : _strand(config.strand),
      _client_token(config.client_token),
      _session(make_ref_<Session>(config.strand, "")),
      _client_connection_maker(config.client_connection_maker) {}

Client::~Client() = default;

void Client::close_impl() {
  if (_connection != nullptr) {
    _connection->close();
  }
}

void Client::connect() {
  if (_connection != nullptr) {
    _connection->close();
    _connection.reset();
  }
  _connection = _client_connection_maker(_strand, _session->session_id(),
                                         _session->last_sent_ack());
  _connection->set_session(_session);
  _connection->connect();
}
}  // namespace dsa

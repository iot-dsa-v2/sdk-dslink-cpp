#include "dsa_common.h"

#include "client.h"
#include "core/session.h"
#include "network/connection.h"

namespace dsa {
Client::Client(WrapperConfig& config)
    : _strand(config.strand),
      _client_token(config.client_token),
      _session(make_ref_<Session>(config.strand, "")) {}

Client::~Client() = default;

void Client::close_impl() { _connection->close(); }
}  // namespace dsa

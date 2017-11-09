#include "dsa_common.h"

#include "client.h"

#include "core/session.h"
#include "module/logger.h"
#include "network/connection.h"

namespace dsa {
Client::Client(WrapperConfig &config)
    : _strand(config.strand),
      _client_token(config.client_token),
      _session(make_ref_<Session>(config.strand, "")),
      _client_connection_maker(config.client_connection_maker),
      _reconnect_timer(config.strand->get_io_service()) {
  _reconnect_timer.async_wait([ this, keep_ref = get_ref() ](
      const boost::system::error_code &error) mutable {
    if (error != boost::asio::error::operation_aborted) {
      _strand->dispatch([ this, keep_ref = std::move(keep_ref) ]() {
        if (is_destroyed()) return;
        reconnect();
      });
    }
  });
}

Client::~Client() = default;

void Client::destroy_impl() {
  if (_connection != nullptr) {
    _connection->destroy();
    _connection.reset();
  }
  _reconnect_timer.cancel();
  _user_on_connect = nullptr;
}

void Client::connect(OnConnectCallback &&on_connect, uint8_t callback_type) {
  if (is_destroyed()) return;

  if (_connection != nullptr) {
    LOG_FATAL(LOG << "Client::connect error, the connection already exists.");
  }
  if (on_connect != nullptr) {
    _user_on_connect = std::move(on_connect);
    _user_on_connect_type = callback_type;
  }
  _session->set_on_connect([ this, keep_ref = get_ref() ](
      const shared_ptr_<Connection> &connection) { _on_connect(connection); });

  _connection = _client_connection_maker(_strand, _session->session_id(),
                                         _session->last_sent_ack());
  _connection->set_session(_session);
  _connection->connect();
}

void Client::_on_connect(const shared_ptr_<Connection> &connection) {
  if (is_destroyed()) return;

  if (_user_on_connect != nullptr) {
    if (connection != nullptr) {
      if (!_last_remote_dsid.empty() &&
          _last_remote_dsid != connection->get_remote_path()) {
        // remote dsid should not change
        LOG_WARN(_strand->logger(),
                 LOG << "remote dsid changed from " << _last_remote_dsid
                     << " to " << connection->get_remote_path());
      }
      if (_user_on_connect_type | BROKER_INFO_CHANGE) {
        if (_last_remote_dsid != connection->get_dsid() ||
            _last_remote_path != connection->get_remote_path()) {
          _user_on_connect(connection);
        }
      } else if (_user_on_connect_type | FIRST_CONNECTION) {
        _user_on_connect_type ^= FIRST_CONNECTION;
        _user_on_connect(connection);
      } else if (_user_on_connect_type | EVERY_CONNECTION) {
        _user_on_connect(connection);
      }
      _last_remote_dsid = connection->get_dsid();
      _last_remote_path = connection->get_remote_path();
    } else {
      if (_user_on_connect_type | DISCONNECTION) {
        _user_on_connect(connection);
      }
      _reconnect_timer.expires_from_now(boost::posix_time::seconds(5));
    }
  }
}
void Client::reconnect() {
  if (is_destroyed()) return;

  if (_connection != nullptr) {
    _connection->destroy();
    _connection.reset();
  }
  _connection = _client_connection_maker(_strand, _session->session_id(),
                                         _session->last_sent_ack());
  _connection->set_session(_session);
  _connection->connect();
}
}  // namespace dsa

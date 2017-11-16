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
      _reconnect_timer(config.strand->get_io_service()) {}

Client::~Client() = default;

void Client::destroy_impl() {
  if (_connection != nullptr) {
    _connection->destroy();
    _connection.reset();
  }
  _session->destroy();
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

  make_new_connection();
}

void Client::_on_connect(const shared_ptr_<Connection> &connection) {
  if (is_destroyed()) return;

  if (connection != nullptr) {
    if (!_last_remote_dsid.empty() && !connection->get_remote_dsid().empty() &&
        _last_remote_dsid != connection->get_remote_dsid()) {
      // remote dsid should not change
      LOG_WARN(_strand->logger(),
               LOG << "remote dsid changed from " << _last_remote_dsid << " to "
                   << connection->get_remote_path());
    }
    _last_connected_time = std::time(nullptr);
    if (_reconnect_interval_s > 0) {
      // set reconnect interval to negative as a flag
      // so during next check we know the dslink is just connected
      _reconnect_interval_s = -_reconnect_interval_s;
    }
    if (_user_on_connect != nullptr) {
      if (_user_on_connect_type & BROKER_INFO_CHANGE) {
        if (_last_remote_dsid != connection->get_dsid() ||
            _last_remote_path != connection->get_remote_path()) {
          _user_on_connect(connection);
        }
      } else if (_user_on_connect_type & FIRST_CONNECTION) {
        _user_on_connect_type ^= FIRST_CONNECTION;
        _user_on_connect(connection);
      } else if (_user_on_connect_type & EVERY_CONNECTION) {
        _user_on_connect(connection);
      }
    }
    _last_remote_dsid = connection->get_remote_dsid();
    _last_remote_path = connection->get_remote_path();
  } else {
    if (_user_on_connect != nullptr &&
        (_user_on_connect_type & DISCONNECTION)) {
      _user_on_connect(connection);
    }

    // check reconnect interval
    if (_reconnect_interval_s < 0) {
      if (std::time(nullptr) - _last_connected_time > 60) {
        // connected for long enough, reset the reconnect timer to 1 second
        _reconnect_interval_s = 1;
      } else {
        // disconnect right after connection, so ++ the previous interval
        _reconnect_interval_s = -_reconnect_interval_s;
      }
    }

    _reconnect();
    if (_reconnect_interval_s < 60) {
      ++_reconnect_interval_s;
    }
  }
}
void Client::_reconnect() {
  LOG_DEBUG(_strand->logger(),
            LOG << "Disconnected, reconnect in " << _reconnect_interval_s
                << " seconds");

  _reconnect_timer.expires_from_now(
      boost::posix_time::seconds(_reconnect_interval_s));
  _reconnect_timer.async_wait([ this, keep_ref = get_ref() ](
      const boost::system::error_code &error) mutable {
    if (error != boost::asio::error::operation_aborted) {
      _strand->dispatch([ this, keep_ref = std::move(keep_ref) ]() {
        if (is_destroyed()) return;
        make_new_connection();
      });
    }
  });
}

void Client::make_new_connection() {
  if (_connection != nullptr) {
    _connection->destroy();
    _connection.reset();
  }
  _connection = _client_connection_maker(_strand, _session->session_id(),
                                         _session->last_sent_ack());
  _connection->set_session(_session);

  _connection->connect(_reconnect_interval_s);
}
}  // namespace dsa

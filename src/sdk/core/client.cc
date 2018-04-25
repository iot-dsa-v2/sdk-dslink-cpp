#include "dsa_common.h"

#include "client.h"

#include "core/session.h"
#include "core/strand_timer.h"
#include "module/logger.h"
#include "network/connection.h"

namespace dsa {
Client::Client(WrapperStrand &config)
    : _strand(config.strand),
      _shared_strand(share_strand_(_strand)),
      _client_token(config.client_token),
      _session(make_ref_<Session>(
          config.strand, config.strand->ecdh().get_dsid(config.dsid_prefix))),
      _client_connection_maker(config.client_connection_maker),
      _reconnect_timer() {
  _session->client_token = config.client_token;
}

Client::~Client() = default;

void Client::destroy_impl() {
  _session->destroy();
  _session.reset();
  if (_connection != nullptr) {
    _connection->destroy();
    _connection.reset();
  }
  if (_reconnect_timer != nullptr) {
    _reconnect_timer->destroy();
    _reconnect_timer.reset();
  }
  _user_on_connect = nullptr;
}

void Client::connect(OnConnectCallback &&on_connect, uint8_t callback_type) {
  if (is_destroyed()) return;

  if (_connection != nullptr) {
    LOG_FATAL(__FILENAME__,
              LOG << "Client::connect error, the connection already exists.");
  }
  if (on_connect != nullptr) {
    _user_on_connect = std::move(on_connect);
    _user_on_connect_type = callback_type;
  }
  _session->set_on_connect([ this, keep_ref = get_ref() ](
      Session & session, const shared_ptr_<Connection> &connection) {
    if (!session.is_destroyed()) {
      _on_connect(connection);
    }
  });

  make_new_connection();
}

void Client::_on_connect(const shared_ptr_<Connection> &connection) {
  if (is_destroyed()) return;

  if (connection != nullptr) {
    if (!_last_remote_dsid.empty() && !connection->get_remote_dsid().empty() &&
        _last_remote_dsid != connection->get_remote_dsid()) {
      // remote dsid should not change
      LOG_WARN(__FILENAME__, LOG << "remote dsid changed from "
                                 << _last_remote_dsid << " to "
                                 << connection->get_remote_path());
    }
    _last_connected_time = std::time(nullptr);
    if (_reconnect_interval_s > 0) {
      // set reconnect interval to negative as a flag
      // so during next check we know the dslink is just connected
      _reconnect_interval_s = -_reconnect_interval_s;
    }
    bool user_on_connect_needed = false;
    if (_user_on_connect != nullptr) {
      if (_user_on_connect_type & BROKER_INFO_CHANGE) {
        if (_last_remote_dsid != connection->get_dsid() ||
            _last_remote_path != connection->get_remote_path()) {
          user_on_connect_needed = true;
        }
      } else if (_user_on_connect_type & FIRST_CONNECTION) {
        _user_on_connect_type ^= FIRST_CONNECTION;
        user_on_connect_needed = true;
      } else if (_user_on_connect_type & EVERY_CONNECTION) {
        user_on_connect_needed = true;
      }
    }
    _last_remote_dsid = connection->get_remote_dsid();
    _last_remote_path = connection->get_remote_path();
    if (user_on_connect_needed) {
      _user_on_connect(connection);
    }
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
  LOG_DEBUG(__FILENAME__, LOG << "Disconnected, reconnect in "
                              << _reconnect_interval_s << " seconds");
  _reconnect_timer =
      _strand->add_timer(_reconnect_interval_s * 1000,
                         [ this, keep_ref = get_ref() ](bool canceled) {
                           if (!(is_destroyed() || canceled)) {
                             make_new_connection();
                           }
                           return false;
                         });
}

void Client::make_new_connection() {
  if (_connection != nullptr) {
    _connection->destroy();
    _connection.reset();
  }
  _connection = _client_connection_maker(_shared_strand);
  _connection->set_session(_session);

  _connection->connect(_reconnect_interval_s);
}
}  // namespace dsa

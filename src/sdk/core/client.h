#ifndef DSA_SDK_CORE_CLIENT_H
#define DSA_SDK_CORE_CLIENT_H

#if defined(_MSC_VER)
#pragma once
#endif

#include "editable_strand.h"
#include "util/enable_ref.h"

namespace dsa {

class Connection;
class Session;

class Client : public DestroyableRef<Client> {
 public:
  typedef std::function<void(const shared_ptr_<Connection> &)>
      OnConnectCallback;

  enum : uint8_t {
    FIRST_CONNECTION = 1,
    BROKER_INFO_CHANGE = 2,
    EVERY_CONNECTION = 4,
    DISCONNECTION = 64,
  };

 protected:
  // for Session/Requester/Responder

  // for ClientConnection
  LinkStrandRef _strand;
  SharedLinkStrandRef _shared_strand;

  string_ _dsid_prefix;
  string_ _client_token;

  ClientConnectionMaker _client_connection_maker;

  shared_ptr_<Connection> _connection;
  ref_<Session> _session;

  void destroy_impl() override;

  // on connect callback related fields

  OnConnectCallback _user_on_connect;
  uint8_t _user_on_connect_type;
  string_ _last_remote_dsid;
  string_ _last_remote_path;

  void _on_connect(const shared_ptr_<Connection> &connection);

  // reconnection related fields

  int64_t _last_connected_time = 0;

  // initially reconnect interval
  int _reconnect_interval_s = 1;

  TimerRef _reconnect_timer;
  void _reconnect();
  void make_new_connection();

 public:
  Client(WrapperStrand &config);

  ~Client();

  Session &get_session() { return *_session; };

  LinkStrand &get_strand() const { return *_strand; }
  const string_ &get_dsid_prefix() const { return _dsid_prefix; }
  const string_ &get_client_token() const { return _client_token; }

  void connect(OnConnectCallback &&on_connect = nullptr,
               uint8_t callback_type = FIRST_CONNECTION);
};

}  // namespace dsa

#endif  // DSA_SDK_CORE_CLIENT_H

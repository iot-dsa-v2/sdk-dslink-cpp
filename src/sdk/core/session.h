#ifndef DSA_SDK_SESSION_H
#define DSA_SDK_SESSION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <deque>
#include <unordered_map>

#include "network/connection.h"
#include "util/enable_shared.h"

#include "client_info.h"
#include "requester/requester.h"
#include "responder/responder.h"
#include "strand_timer.h"

namespace dsa {
class MessageStream;
class AckStream;
class PingStream;
class IncomingMessageStream;
class OutgoingMessageStream;
class Connection;
class SessionManager;

struct AckHolder {
  int32_t ack;
  AckCallback callback;
  AckHolder(int32_t ack, AckCallback &&callback)
      : ack(ack), callback(std::move(callback)){};
};

class Session;

// maintain request and response streams
class Session final : public DestroyableRef<Session> {
  friend class Connection;
  friend class Responder;
  friend class MessageStream;

 public:
  typedef std::function<void(const ref_<Session> &session,
                             const ClientInfo &info)>
      GetSessionCallback;

  typedef std::function<void(Session &, const shared_ptr_<Connection> &)>
      OnConnectCallback;

 public:
  // call back on connect
  void set_on_connect(OnConnectCallback &&callback);

 private:
  int32_t _waiting_ack = 0;

  std::deque<AckHolder> _pending_acks;
  void check_pending_acks(int32_t ack);

  string_ _remote_id;
  string_ _log_id;
  shared_ptr_<Connection> _connection;
  bool _reconnection_expired = false;

  ref_<AckStream> _ack_stream;
  ref_<PingStream> _ping_stream;
  std::deque<ref_<MessageStream>> _write_streams;
  bool _is_writing = false;

  LinkStrandRef _strand;

  ref_<MessageStream> get_next_ready_stream(int64_t time);

  size_t peek_next_message(size_t available, int64_t time);

  static void write_loop(ref_<Session> sthis);

  void receive_message(MessageRef &&message);

  OnConnectCallback _on_connect;

  // a timer to check if connection is disconnected and send ping message
  int _no_receive_in_loop = 0;
  int _no_sent_in_loop = 0;
  TimerRef _timer;
  // runs every 15 seconds when connected
  // control ping message and connection timeout
  bool _on_timer();

 public:
  Requester requester;
  Responder responder;

  // client session features

  bool responder_enabled = true;
  string_ client_token;

  Session(LinkStrandRef strand, const string_ &dsid);
  Session(LinkStrandRef strand, const string_ &dsid, const string_ &base_path);
  ~Session();

  LinkStrandRef &get_strand() { return _strand; };

  const string_ &get_remote_id() const { return _remote_id; }
  const string_ &get_log_id() const { return _log_id; }

  bool is_connected() const { return _connection != nullptr; }
  bool is_writing() const { return _is_writing; }

  int32_t last_sent_ack();

  void connected(shared_ptr_<Connection> connection);
  void disconnected(const shared_ptr_<Connection> &connection);

  void destroy_impl() final;

  void write_stream(ref_<MessageStream> &&stream);
  // this stream must be handled first
  void write_critical_stream(ref_<MessageStream> &&stream);

 private:
  string_ _base_path;

 public:
  // used by broker to forward the pub path
  string_ map_pub_path(const string_ &path);
};

}  // namespace dsa

#endif  // DSA_SDK_SESSION_H
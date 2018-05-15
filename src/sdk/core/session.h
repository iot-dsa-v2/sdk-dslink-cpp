#ifndef DSA_SDK_SESSION_H
#define DSA_SDK_SESSION_H

#if defined(_MSC_VER)
#pragma once
#endif

#include <deque>
#include <unordered_map>

#include "message/base_message.h"
#include "message/message_options.h"
#include "network/connection.h"
#include "strand_timer.h"
#include "stream/stream_callbacks.h"
#include "util/client_info.h"
#include "util/enable_shared.h"

namespace dsa {
class MessageStream;
class AckStream;
class PingStream;
class IncomingMessageStream;
class OutgoingMessageStream;
class Connection;
class SessionManager;
class OutgoingInvokeStream;
class OutgoingSubscribeStream;
class OutgoingListStream;
class OutgoingSetStream;
class ListRequestMessage;
class SubscribeRequestMessage;

struct AckHolder {
  int32_t ack;
  AckCallback callback;
  AckHolder(int32_t ack, AckCallback &&callback)
      : ack(ack), callback(std::move(callback)){};
};

class StreamManager : public DestroyableRef<StreamManager> {
 public:
  virtual void write_stream(ref_<MessageStream> &&stream) = 0;
  // this stream must be handled first
  virtual void write_critical_stream(ref_<MessageStream> &&stream) = 0;

  virtual bool destroy_resp_stream(int32_t rid) = 0;
  virtual bool destroy_req_stream(int32_t rid) = 0;
};

// maintain request and response streams
class Session final : public StreamManager {
  friend class Connection;

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
  string_ _role;
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
  // client session features

  bool responder_enabled = true;
  string_ client_token;

  Session(const LinkStrandRef &strand, const string_ &dsid,
          const string_ &role);
  Session(const LinkStrandRef &strand, const string_ &dsid, const string_ &role,
          const string_ &base_path);
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

  void write_stream(ref_<MessageStream> &&stream) final;
  // this stream must be handled first
  void write_critical_stream(ref_<MessageStream> &&stream) final;

 private:
  string_ _base_path;

 public:
  // used by broker to forward the pub path
  string_ map_pub_path(const string_ &path);

  virtual bool destroy_resp_stream(int32_t rid);
  virtual bool destroy_req_stream(int32_t rid);

 protected:  // responder
  std::unordered_map<int32_t, ref_<MessageStream>> _outgoing_streams;

  ref_<OutgoingInvokeStream> on_invoke_request(
      ref_<InvokeRequestMessage> &&request);
  ref_<OutgoingListStream> on_list_request(ref_<ListRequestMessage> &&request);
  ref_<OutgoingSetStream> on_set_request(ref_<SetRequestMessage> &&request);
  ref_<OutgoingSubscribeStream> on_subscribe_request(
      ref_<SubscribeRequestMessage> &&request);

  void receive_resp_message(ref_<Message> &&message);
  void responder_disconnected();

 protected:  // requester
  int32_t _next_rid = 0;
  int32_t next_rid();

  std::unordered_map<int32_t, ref_<MessageStream>> _incoming_streams;

  void receive_req_message(ref_<Message> &&message);
  void requester_disconnected();

 public:  // requester
  ref_<IncomingSubscribeStream> subscribe(
      const string_ &path, IncomingSubscribeStreamCallback &&callback,
      const SubscribeOptions &options = SubscribeOptions::default_options);

  ref_<IncomingListStream> list(
      const string_ &path, IncomingListStreamCallback &&callback,
      const ListOptions &options = ListOptions::default_options);

  ref_<IncomingInvokeStream> invoke(IncomingInvokeStreamCallback &&callback,
                                    ref_<const InvokeRequestMessage> &&message);

  ref_<IncomingSetStream> set(IncomingSetStreamCallback &&callback,
                              ref_<const SetRequestMessage> &&message);
};

}  // namespace dsa

#endif  // DSA_SDK_SESSION_H
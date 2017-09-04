#ifndef DSA_SDK_SESSION_H_
#define DSA_SDK_SESSION_H_

#include <deque>
#include <map>

#include "network/connection.h"
#include "util/enable_shared.h"

#include "requester/requester.h"
#include "responder/responder.h"

namespace dsa {
class MessageStream;
class IncomingMessageStream;
class OutgoingMessageStream;
class Connection;

//////////////////////////////////////////
// maintain request and response streams
//////////////////////////////////////////
class Session final : public ClosableRef<Session> {
  friend class Connection;
  friend class Responder;
  friend class MessageStream;

 private:
  int32_t  _next_ack = 0;
  std::string _dsid;
  std::string _session_id;
  shared_ptr_<Connection> _connection;

  std::deque<ref_<MessageStream> > _write_streams;
  bool _is_writing = false;

  LinkStrandRef _strand;

  ref_<MessageStream> get_next_ready_stream();

  size_t peek_next_message(size_t availible);

  static void write_loop(ref_<Session> sthis);

  void receive_message(MessageRef &&message);

 public:
  Requester requester;
  Responder responder;

  Session(LinkStrandRef &strand, const std::string &session_id);

  const std::string &dsid() const { return _dsid; }
  const std::string &session_id() const { return _session_id; }
  bool is_connected() const { return _connection != nullptr; }

  void connected(shared_ptr_<Connection> connection);
  void disconnected(const shared_ptr_<Connection> &connection);

  void close_impl() final;

  void write_stream(ref_<MessageStream> &&stream);
};

}  // namespace dsa

#endif  // DSA_SDK_SESSION_H_
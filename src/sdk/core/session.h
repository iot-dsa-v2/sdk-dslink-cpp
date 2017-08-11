#ifndef DSA_SDK_SESSION_H_
#define DSA_SDK_SESSION_H_

#include <atomic>
#include <map>
#include <queue>

#include <boost/asio.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "util/enable_shared.h"
#include "connection.h"

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
class Session : public IntrusiveClosable<Session> {
 public:
  struct StreamInfo {
    uint32_t rid;
    size_t unique_id;
    std::map<uint32_t, shared_ptr_<MessageStream>> *container;
  };

  Requester requester;
  Responder responder;



  explicit Session(boost::asio::io_service::strand &strand,
                   const std::string &session_id,
                   const shared_ptr_<Connection> &connection = nullptr);

  const std::string &session_id() const { return _session_id; }

  void start() const;

  void close();

  void set_connection(const shared_ptr_<Connection> &connection) { _connection = connection; };

  boost::asio::io_service::strand &strand() { return _strand; };
  const boost::asio::io_service::strand &strand() const { return _strand; }

 private:
  static std::atomic_size_t _session_count;
  std::string _session_id;
  shared_ptr_<Connection> _connection;
  
  std::map<uint32_t, shared_ptr_<MessageStream>> _incoming_streams;
  boost::shared_mutex _outgoing_key;
  boost::shared_mutex _incoming_key;
  std::queue<StreamInfo> _ready_streams;
  std::atomic_bool _is_writing{false};
  boost::asio::io_service::strand &_strand;

  MessageStream *_get_next_ready_stream();
  void _write_loop();

  friend class Connection;
  void connection_closed();
};

typedef shared_ptr_<Session> SessionPtr;

}  // namespace dsa

#endif // DSA_SDK_SESSION_H_
#ifndef DSA_SDK_SESSION_H_
#define DSA_SDK_SESSION_H_

#include <atomic>
#include <map>
#include <queue>

#include <boost/asio.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "util/enable_shared.h"
#include "network/connection.h"

#include "requester/requester.h"
#include "responder/responder.h"


namespace dsa {
class MessageStream;
class IncomingMessageStream;
class OutgoingMessageStream;

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

  void add_ready_outgoing_stream(uint32_t rid, size_t unique_id);

  explicit Session(boost::asio::io_service::strand &strand,
                   BufferPtr session_id,
                   const ConnectionPtr &connection = nullptr);
  explicit Session(boost::asio::io_service::strand &strand,
                   const std::string &session_id,
                   ConnectionPtr connection = nullptr);

  const BufferPtr &session_id() const { return _session_id; }

  void start() const;

  void close();

  void set_connection(const ConnectionPtr &connection) { _connection = connection; };

  bool add_outgoing_subscription(const shared_ptr_<OutgoingMessageStream> &stream);

  void remove_outgoing_subscription(uint32_t request_id);

  boost::asio::io_service::strand &strand() { return _strand; };
  const boost::asio::io_service::strand &strand() const { return _strand; }

 private:
  static std::atomic_size_t _session_count;
  BufferPtr _session_id;
  ConnectionPtr _connection;
  std::map<uint32_t, shared_ptr_<MessageStream>> _outgoing_streams;
  std::map<uint32_t, shared_ptr_<MessageStream>> _incoming_streams;
  boost::shared_mutex _outgoing_key;
  boost::shared_mutex _incoming_key;
  std::queue<StreamInfo> _ready_streams;
  std::atomic_bool _is_writing{false};
  boost::asio::io_service::strand &_strand;

  MessageStream *_get_next_ready_stream();
  void _write_loop();
};

typedef shared_ptr_<Session> SessionPtr;

}  // namespace dsa

#endif // DSA_SDK_SESSION_H_
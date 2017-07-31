#ifndef DSA_SDK_SESSION_H_
#define DSA_SDK_SESSION_H_

#include <atomic>
#include <map>
#include <queue>

#include <boost/asio.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "app.h"
#include "util/enable_shared.h"

namespace dsa {
class MessageStream;
class IncomingMessageStream;
class OutgoingMessageStream;

//////////////////////////////////////////
// maintain request and response streams
//////////////////////////////////////////
class Session: public InheritableEnableShared<Session> {
 public:
  struct StreamInfo {
    uint32_t rid;
    size_t unique_id;
    std::map<uint32_t, std::shared_ptr<MessageStream>> *container;
  };
  void add_ready_outgoing_stream(uint32_t rid, size_t unique_id);

  explicit Session(BufferPtr session_id, const ConnectionPtr &connection = nullptr);
  explicit Session(const std::string &session_id, ConnectionPtr connection = nullptr);

  const BufferPtr &session_id() const { return _session_id; }

  void start() const;

  void stop();

  void set_connection(const ConnectionPtr &connection) { _connection = connection; };

  bool add_outgoing_subscription(const std::shared_ptr<OutgoingMessageStream> &stream);

  boost::asio::io_service::strand &strand() { return *_strand; };

 private:
  static std::atomic_size_t _session_count;
  std::atomic_size_t _stream_count;
  BufferPtr _session_id;
  ConnectionPtr _connection;
  std::map<uint32_t, std::shared_ptr<MessageStream>> _outgoing_streams;
  std::map<uint32_t, std::shared_ptr<MessageStream>> _incoming_streams;
  boost::shared_mutex _outgoing_key;
  boost::shared_mutex _incoming_key;
  std::queue<StreamInfo> _ready_streams;
  std::atomic_bool _is_writing{false};

  MessageStream *_get_next_ready_stream();
  void _write_loop();

  std::unique_ptr<boost::asio::io_service::strand> _strand;
};

typedef std::shared_ptr<Session> SessionPtr;

}  // namespace dsa

#endif // DSA_SDK_SESSION_H_
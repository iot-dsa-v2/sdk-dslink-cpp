#ifndef DSA_SDK_SESSION_H_
#define DSA_SDK_SESSION_H_

#include <atomic>
#include <map>
#include <queue>

#include <boost/asio.hpp>
#include <boost/thread/shared_mutex.hpp>

#include "app.h"
#include "responder/outgoing_message_stream.h"
#include "requester/incoming_message_stream.h"

namespace dsa {

//////////////////////////////////////////
// maintain request and response streams
//////////////////////////////////////////
class Session: public std::enable_shared_from_this<Session> {
 private:
  static std::atomic_long _session_count;
  std::atomic_long _stream_count;
  BufferPtr _session_id;
  ConnectionPtr _connection;
  std::map<uint32_t, std::shared_ptr<OutgoingMessageStream>> _outgoing_streams;
  std::queue<unsigned int> _ready_streams;

  friend class OutgoingMessageStream;
  std::unique_ptr<boost::asio::io_service::strand> _strand;
  void add_ready_stream(unsigned int stream_id);

 public:
  explicit Session(BufferPtr session_id, const ConnectionPtr &connection = nullptr);
  explicit Session(const std::string &session_id, ConnectionPtr connection = nullptr);

  const BufferPtr &session_id() const { return _session_id; }

  void start() const;

  void stop();

  void set_connection(const ConnectionPtr &connection) { _connection = connection; };

};

typedef std::shared_ptr<Session> SessionPtr;

}  // namespace dsa

#endif // DSA_SDK_SESSION_H_
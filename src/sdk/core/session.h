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

  void close() override;

  void set_connection(const shared_ptr_<Connection> &connection) { _connection = connection; };

  boost::asio::io_service::strand &strand() { return _strand; };
  const boost::asio::io_service::strand &strand() const { return _strand; }

 private:
  std::string _session_id;
  shared_ptr_<Connection> _connection;
  

  std::queue<StreamInfo> _ready_streams;
  bool _is_writing{false};
  boost::asio::io_service::strand &_strand;

  MessageStream *get_next_ready_stream();
  void write_loop();

  friend class Connection;
  void connection_closed();
  void receive_message(Message * message);
};


}  // namespace dsa

#endif // DSA_SDK_SESSION_H_
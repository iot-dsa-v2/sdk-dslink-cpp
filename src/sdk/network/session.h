#ifndef DSA_SDK_SESSION_H_
#define DSA_SDK_SESSION_H_

#include <memory>
#include <atomic>

#include <boost/asio.hpp>

#include "app.h"

namespace dsa {

//////////////////////////////////////////
// maintain request and response streams
//////////////////////////////////////////
class Session: public std::enable_shared_from_this<Session> {
 private:
  static std::atomic_long _session_count;
  std::unique_ptr<boost::asio::io_service::strand> _strand;
  BufferPtr _session_id;
  ConnectionPtr _connection;

  void message_handler(SharedBuffer message_buffer) const;

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
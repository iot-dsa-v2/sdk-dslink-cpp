#ifndef DSA_SDK_SESSION_H_
#define DSA_SDK_SESSION_H_

#include <memory>
#include <atomic>

#include <boost/asio.hpp>

#include "app.h"

namespace dsa {
// maintain request and response streams
class Session: public EnableShared<Session> {
 private:
  static std::atomic_long _session_count;
  std::unique_ptr<boost::asio::io_service::strand> _strand;
  std::shared_ptr<Connection> _connection;
  long _session_id;

 public:
  explicit Session(std::shared_ptr<Connection> connection);

  long session_id() const { return _session_id; }

  void start() const;

};
}  // namespace dsa

#endif // DSA_SDK_SESSION_H_
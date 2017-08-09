#ifndef DSA_SDK_NETWORK_SERVER_H
#define DSA_SDK_NETWORK_SERVER_H

#include <atomic>
#include <functional>
#include <iostream>
#include <map>
#include <string>
#include <utility>

#include "config.h"
#include "session.h"
#include "session_manager.h"
#include "util/buffer.h"
#include "util/enable_shared.h"

namespace dsa {
class Connection;
class SessionManager;

typedef std::function<void(const intrusive_ptr_<Session> &,
                           Buffer::SharedBuffer)>
    MessageHandler;
typedef std::function<void(const intrusive_ptr_<Session> &)> OnConnectHandler;

class Server : public SharedClosable<Server> {
 private:
  SessionManager _session_manager;

 protected:
  boost::asio::io_service::strand &_strand;
  const Config _config;

  void on_session_connected(const shared_ptr_<Session> &session);

 public:
  SessionManager &session_manager() { return _session_manager; }

  enum Protocol { TCP };

  explicit Server(boost::asio::io_service::strand &strand, const Config &config);

  virtual void start() = 0;
  void close() override;
  virtual std::string type() = 0;
};

typedef shared_ptr_<Server> ServerPtr;

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SERVER_H

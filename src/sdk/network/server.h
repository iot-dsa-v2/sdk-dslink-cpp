#ifndef  DSA_SDK_NETWORK_SERVER_H
#define  DSA_SDK_NETWORK_SERVER_H

#include <functional>
#include <iostream>
#include <string>
#include <atomic>
#include <utility>
#include <map>

#include "gracefully_closable.h"
#include "util/buffer.h"

namespace dsa {
class SessionManager;
class Connection;
class Session;

typedef std::function<void (const shared_ptr_<Session> &, Buffer::SharedBuffer)> MessageHandler;
typedef std::function<void (const shared_ptr_<Session> &)> OnConnectHandler;

class Server : public GracefullyClosable {
 private:
  shared_ptr_<SessionManager> _session_manager;

 public:
  shared_ptr_<SessionManager> session_manager() { return _session_manager; }

  enum Protocol {
    TCP
  };

  class Config {
   private:
    std::string _path{"/"};
    unsigned short _port{8080};
    MessageHandler _message_handler{[](shared_ptr_<Session> s, Buffer::SharedBuffer b){}};
    OnConnectHandler _on_connect{[](const shared_ptr_<Session> &s){}};

   public:
    Config(std::string path, unsigned short port) : _path(std::move(path)), _port(port) {}
    Config() = default;
    ~Config() = default;
    Config(Config&&) = default;
    Config(const Config &) = default;
    Config &operator=(Config&&) = default;
    Config &operator=(const Config &) = default;

    void set_port(unsigned short port) { _port = port; }
    void set_path(const char *path) { _path = path; }
    void set_message_handler(MessageHandler message_handler) { _message_handler = std::move(message_handler); }
    void set_on_connect(OnConnectHandler on_connect) { _on_connect = std::move(on_connect); }

    unsigned short port() const { return _port; }
    const std::string &path() const { return _path; }
    MessageHandler message_handler() const { return _message_handler; }
    const OnConnectHandler &on_connect() const { return _on_connect; }
  };

  explicit Server(shared_ptr_<App> app);

  virtual void start() = 0;
  void stop() override;
  virtual std::string type() = 0;
};

typedef shared_ptr_<Server> ServerPtr;

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SERVER_H

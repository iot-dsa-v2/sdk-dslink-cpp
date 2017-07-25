#ifndef  DSA_SDK_NETWORK_SERVER_H
#define  DSA_SDK_NETWORK_SERVER_H

#include <functional>
#include <iostream>
#include <string>
#include <atomic>
#include <utility>
#include <map>
#include <memory>

#include "gracefully_closable.h"
#include "util/buffer.h"

namespace dsa {
class SessionManager;
class Connection;
class Session;

typedef std::function<void (std::shared_ptr<Session>, Buffer::SharedBuffer)> MessageHandler;

class Server : public GracefullyClosable {
 private:
  std::shared_ptr<SessionManager> _session_manager;

 public:
  std::shared_ptr<SessionManager> session_manager() { return _session_manager; }

  enum Protocol {
    TCP
  };

  class Config {
   private:
    std::string _path{"/"};
    unsigned short _port{8080};
    MessageHandler _message_handler{[](std::shared_ptr<Session> s, Buffer::SharedBuffer b){}};

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

    unsigned short port() const { return _port; }
    const std::string &path() const { return _path; }
    MessageHandler message_handler() const { return _message_handler; }
  };

  explicit Server(std::shared_ptr<App> app);

  virtual void start() = 0;
  void stop() override;
  virtual std::string type() = 0;
};

typedef std::shared_ptr<Server> ServerPtr;

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SERVER_H

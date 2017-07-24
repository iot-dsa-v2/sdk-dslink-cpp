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

namespace dsa {
class SessionManager;
class Connection;

class Server : public GracefullyClosable {
 private:
  std::shared_ptr<SessionManager> _session_manager;

 public:
  std::shared_ptr<SessionManager> session_manager() { return _session_manager; }

  enum Type {
    TCP
  };

  class Config {
   private:
    std::string _path{"/"};
    unsigned short _port{8080};

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

    unsigned short port() const { return _port; }
    const std::string &path() const { return _path; }
  };

  explicit Server(App &app);

  virtual void start() = 0;
  virtual void stop();
  virtual std::string type() = 0;

  void operator()() override { stop(); }
};

typedef std::shared_ptr<Server> ServerPtr;

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SERVER_H

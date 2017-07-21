#ifndef  DSA_SDK_NETWORK_SERVER_H
#define  DSA_SDK_NETWORK_SERVER_H

#include <atomic>
#include <utility>
#include <map>

#include "util/enable_shared.h"
#include "security_context.h"
#include "app.h"

namespace dsa {
class Session;

class Server : public InheritableEnableShared<Server>, virtual public AppClosable {
 protected:
  std::map<std::string, std::shared_ptr<Session>> _sessions;
  std::atomic_long _session_count{0};

 public:
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

  explicit Server(App &app) : AppClosable(app) {}
  virtual std::shared_ptr<Session> get_session(const std::string &session_id) = 0;
  virtual std::shared_ptr<Session> create_session() = 0;
  virtual std::string get_new_session_id() = 0;

  virtual void start() = 0;
  virtual void stop();
  virtual std::string type() = 0;
};

typedef std::shared_ptr<Server> ServerPtr;

}  // namespace dsa

#endif  // DSA_SDK_NETWORK_SERVER_H

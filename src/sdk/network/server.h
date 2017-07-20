#ifndef  DSA_SDK_SERVER_H
#define  DSA_SDK_SERVER_H

#include "security_context.h"
#include "util/enable_shared.h"

namespace dsa {
class App;

class Server : public InheritableEnableShared<Server> {
 protected:
  const App &_app;

 public:
  enum Type {
    TCP
  };

  class Config {
   private:
    unsigned short _port{8080};
    std::string _path{"/example/path"};

   public:
    void set_port(unsigned short port) { _port = port; }
    void set_path(const char *path) { _path = path; }
    unsigned short port() const { return _port; }
    const std::string &path() const { return _path; }
  };

  explicit Server(const App &app);
  virtual void start() = 0;
  void stop();
  virtual std::string type() = 0;
};

typedef std::shared_ptr<Server> ServerPtr;

}  // namespace dsa

#endif  // DSA_SDK_SERVER_H

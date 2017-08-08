#ifndef DSA_SDK_CORE_CLIENT_H_
#define DSA_SDK_CORE_CLIENT_H_

#include "config.h"

namespace dsa {

class App;
class Session;
class Connection;

class Client {
 protected:
  shared_ptr_<Session> _session;
  shared_ptr_<Connection> _connection;

  const App *_app;
  const Config config;
  

 public:
  Client(const App &app, const Config &config);
};

}  // namespace dsa

#endif  // DSA_SDK_CORE_CLIENT_H_

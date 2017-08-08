#ifndef DSA_SDK_CORE_CLIENT_H_
#define DSA_SDK_CORE_CLIENT_H_

#include "config.h"

#include "util/enable_shared.h"

namespace dsa {

class App;
class Session;
class Connection;

class Client : public GracefullyClosable<Client> {
 protected:
  shared_ptr_<Session> _session;
  shared_ptr_<Connection> _connection;

  const App *_app;
  const Config config;

 public:
  Client(const App &app, const Config &config);

  virtual void connect() = 0;

  void close() override;
};

}  // namespace dsa

#endif  // DSA_SDK_CORE_CLIENT_H_

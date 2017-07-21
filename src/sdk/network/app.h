//
// Created by Ben Richards on 7/17/17.
//

#ifndef DSA_SDK_NETWORK_APP_H_
#define DSA_SDK_NETWORK_APP_H_

#include <memory>
#include <string>
#include <mutex>
#include <unordered_map>

#include "server.h"
#include "client.h"
#include "security_context.h"
#include "util/enable_shared.h"

namespace boost {
class thread_group;
namespace asio {
class io_service;
}
}

namespace dsa {
class io_service_work;
class GracefullyClosable;

class App {
 private:
  std::shared_ptr<boost::asio::io_service> _io_service;
  std::shared_ptr<io_service_work> _work;
  std::shared_ptr<SecurityContext> _security_context;
  std::shared_ptr<boost::thread_group> _threads;
  std::string _name;
  std::mutex _register_key;

 protected:
  //////////////////////////////////////////////////
  // GracefullyClosable components only get access to the
  // register and un-register functions
  //////////////////////////////////////////////////
  std::unordered_map<void *, std::weak_ptr<GracefullyClosable>> _registry;
  friend class GracefullyClosable;

  // register new component
  void register_component(std::shared_ptr<GracefullyClosable> component);
  // un-register dead component
  void unregister_component(void *component);

 public:
  explicit App(std::string name);
  App(std::string name, std::shared_ptr<boost::asio::io_service> io_service);
  boost::asio::io_service &io_service() const { return *_io_service; };
  SecurityContext &security_context() const { return *_security_context; };
  const std::string &name() const { return _name; };

  // start io_service and wait forever
  void run(unsigned int thread_count = 5);

  // start io_service asynchronously and continue
  void async_start(unsigned int thread_count = 5);

  // allows jobs to finish then stops io_service, may not stop if servers or clients are listening
  void graceful_stop();

  // halts jobs and stops io_service
  void stop();

  // wait forever or until all worker threads fail
  void wait();

  // sleep current thread in milliseconds
  void sleep(unsigned int milliseconds);

  // get new server
  std::shared_ptr<Server> new_server(Server::Type type, const Server::Config &config);

  // get new client
  ClientPtr new_client(Client::Type type, const Client::Config &config);
};
}  // namespace dsa

#endif //DSA_SDK_NETWORK_APP_H_

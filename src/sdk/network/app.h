//
// Created by Ben Richards on 7/17/17.
//

#ifndef DSA_SDK_NETWORK_APP_H_
#define DSA_SDK_NETWORK_APP_H_

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

class App : public std::enable_shared_from_this<App> {
 private:
  shared_ptr_<boost::asio::io_service> _io_service;
  shared_ptr_<io_service_work> _work;
  shared_ptr_<SecurityContext> _security_context;
  shared_ptr_<boost::thread_group> _threads;
  std::unique_ptr<boost::asio::io_service::strand> _strand;
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
  void register_component(shared_ptr_<GracefullyClosable> component);
  // un-register dead component
  void unregister_component(void *component);

 public:
  explicit App(std::string name);
  App(std::string name, shared_ptr_<boost::asio::io_service> io_service);
  boost::asio::io_service &io_service() const { return *_io_service; };
  SecurityContext &security_context() const { return *_security_context; };
  const std::string &name() const { return _name; };

  // start io_service and wait forever
  void run(unsigned int thread_count = 5);

  // start io_service asynchronously and continue
  void async_start(unsigned int thread_count = 5);

  // allows jobs to finish then stops io_service, may not stop if servers or clients are listening
  void graceful_stop();

  // allows jobs to finish then stops io_service, hard stops after timeout
  void graceful_stop(unsigned int milliseconds);

  // halts jobs and stops io_service
  void stop();

  // wait forever or until all worker threads fail
  void wait();

  // sleep current thread in milliseconds
  void sleep(unsigned int milliseconds);

  // get new server
  Server *new_server(Server::Protocol type, const Server::Config &config) throw();

  // get new client
  Client *new_client(Client::Protocol type, const Client::Config &config) throw();

  // get strand
  boost::asio::io_service::strand &strand() { return *_strand; }
};
}  // namespace dsa

#endif //DSA_SDK_NETWORK_APP_H_

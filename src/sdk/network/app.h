//
// Created by Ben Richards on 7/17/17.
//

#ifndef DSA_SDK_NETWORK_APP_H_
#define DSA_SDK_NETWORK_APP_H_

#include <atomic>
#include <string>
#include <mutex>
#include <unordered_map>

#include "util/enable_shared.h"
#include "server.h"
#include "client.h"
#include "security_context.h"

namespace boost {
class thread_group;
namespace asio {
class io_service;
}
}

namespace dsa {
class io_service_work;

class App : public std::enable_shared_from_this<App> {
 private:
  shared_ptr_<boost::asio::io_service> _io_service;
  shared_ptr_<io_service_work> _work;
  shared_ptr_<SecurityContext> _security_context;
  shared_ptr_<boost::thread_group> _threads;
  std::unique_ptr<boost::asio::io_service::strand> _strand;
  std::string _name;

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
  void close();

  // halts jobs and stops io_service
  void force_stop();

  // wait forever or until all worker threads fail
  void wait();

  // sleep current thread in milliseconds
  void sleep(unsigned int milliseconds);

  // get strand
  boost::asio::io_service::strand &strand() const { return *_strand; }
};
}  // namespace dsa

#endif //DSA_SDK_NETWORK_APP_H_

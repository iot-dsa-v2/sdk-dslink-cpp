//
// Created by Ben Richards on 7/17/17.
//

#ifndef DSA_SDK_NETWORK_APP_H_
#define DSA_SDK_NETWORK_APP_H_

#include <memory>
#include <string>

namespace boost {
namespace asio {
class io_service;
}
}

namespace dsa {

class SecurityContext;

class App {
 private:
  std::shared_ptr<boost::asio::io_service> _io_service;
  std::shared_ptr<SecurityContext> _security_context;
  std::string _name;

 public:
  App(std::string name);
  App(std::string name, std::shared_ptr<boost::asio::io_service> io_service);
  boost::asio::io_service &io_service() const;
  SecurityContext &security_context() const;
  std::string name() const;
  void run(unsigned int thread_count = 5);
};

}  // namespace dsa

#endif //DSA_SDK_NETWORK_APP_H_

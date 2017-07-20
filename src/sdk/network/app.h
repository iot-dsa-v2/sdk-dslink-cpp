//
// Created by Ben Richards on 7/17/17.
//

#ifndef DSA_SDK_NETWORK_APP_H_
#define DSA_SDK_NETWORK_APP_H_

#include <memory>
#include <string>

#include "server.h"
#include "client.h"
#include "security_context.h"

namespace boost {
namespace asio {
class io_service;
}
}

namespace dsa {

class App {
 private:
  std::shared_ptr<boost::asio::io_service> _io_service;
  std::shared_ptr<SecurityContext> _security_context;
  std::string _name;

  std::vector<ServerPtr> _servers;

 public:
  explicit App(std::string name);
  App(std::string name, std::shared_ptr<boost::asio::io_service> io_service);
  boost::asio::io_service &io_service() const { return *_io_service; };
  SecurityContext &security_context() const { return *_security_context; };
  const std::string &name() const { return _name; };
  void run(unsigned int thread_count = 5);
  void async_run(unsigned int thread_count = 5);
  void add_server(Server::Type type, Server::Config config);
  ClientPtr new_client(Client::Config config);
};
}  // namespace dsa

#endif //DSA_SDK_NETWORK_APP_H_
